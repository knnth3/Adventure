// Fill out your copyright notice in the Description page of Project Settings.
#include "WorldGrid.h"

#include "Spawner.h"
#include "Adventure.h"
#include "PathFinder.h"
#include "Interactable.h"
#include "Character/MapPawn.h"
#include "Character/ConnectedPlayer.h"

#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMaterialLibrary.h"

// Sets default values
AWorldGrid::AWorldGrid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bInitialized = false;
	ActivePlayerID = -1;
	HostID = 0;

	//Create a component to hold the visual
	PlaneVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualRepresentation"));
	RootComponent = PlaneVisual;

	//Load the visual for the grid
	if (PlaneVisual)
	{
		PlaneVisual->SetRelativeLocation(FVector(-500.0f, 500.0f, 0.0f));
		PlaneVisual->SetWorldScale3D(FVector(10.0f));
	}
}

void AWorldGrid::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWorldGrid, Dimensions);
}

void AWorldGrid::Initialize(const int HostID, FGridCoordinate GridDimensions)
{
	if (HasAuthority() && !bInitialized)
	{
		this->HostID = HostID;
		bInitialized = true;
		Dimensions = GridDimensions;
		GenerateGrid();
		SetupVisuals(Dimensions);
	}
}

FGridCoordinate AWorldGrid::GetDimensions()const
{
	return Dimensions;
}

bool AWorldGrid::IsSpaceTraversable(const FGridCoordinate & Location)const
{
	CellPtr CellLocation = At(Location);
	if (CellLocation)
	{
		return CellLocation->bNonTraversable;
	}
	return false;
}

bool AWorldGrid::IsSpawnLocation(const FGridCoordinate & Location)const
{
	return (SpawnLocations.find(Location.toPair()) != SpawnLocations.end());
}

bool AWorldGrid::IsFreeRoamActive() const
{
	return (ActivePlayerID == -1);
}

bool AWorldGrid::IsTurn(const int PawnID) const
{
	return (IsFreeRoamActive() || (PawnID == ActivePlayerID));
}

void AWorldGrid::BeginTurnBasedMechanics(const TArray<int>& Order)
{
	if (Order.Num() > 0)
	{
		for (auto& ID : Order)
		{
			TurnSequence.push(ID);
		}
		for (auto& players : PlayerCollection)
		{
			if (players.first == TurnSequence.front())
			{
				players.second->SetPlayerState(TURN_BASED_STATE::ACTIVE);
			}
			else
			{
				players.second->SetPlayerState(TURN_BASED_STATE::STANDBY);
			}
		}
		ActivePlayerID = TurnSequence.front();
	}
}

void AWorldGrid::EndTurnBasedMechanics()
{
	for (auto& players : PlayerCollection)
	{
		players.second->SetPlayerState(TURN_BASED_STATE::FREE_ROAM);
	}
	ActivePlayerID = -1;
}

void AWorldGrid::EndTurn(const int PawnID)
{
}

bool AWorldGrid::MoveCharacter(const int PawnID, const FGridCoordinate& Destination, const bool bOverrideMechanics)
{
	int OwnerID;
	int PawnIndex;
	GetPawnIDParams(PawnID, OwnerID, PawnIndex);
	auto& Selected = PawnCollection[OwnerID][PawnIndex];
	if (Selected)
	{
		if (bOverrideMechanics || IsTurn(PawnID))
		{
			(*Selected)->SetDestination(Destination);
			return true;
		}
	}

	return false;
}

void AWorldGrid::RegisterPlayerController(AConnectedPlayer* ConnectedPlayer)
{
	int OwnerID = ConnectedPlayer->GetPlayerID();
	PlayerCollection[OwnerID] = ConnectedPlayer;
	ConnectedPlayer->SetPlayerState(TURN_BASED_STATE::FREE_ROAM);
}

int AWorldGrid::AddCharacter(int OwnerID, bool OverrideLocation, FVector NewLocation, int ClassIndex)
{
	int PawnIndex = GenerateNewPawnIndex(OwnerID);
	int PawnID = (OwnerID * 10) + PawnIndex;

	if (ClassIndex >= 0 && ClassIndex < MapPawnClasses.Num() && MapPawnClasses[ClassIndex])
	{
		std::shared_ptr<FGridCoordinate> Location = (OverrideLocation) ? std::make_shared<FGridCoordinate>(NewLocation) : GetOpenSpawnLocation();
		if (Location)
		{
			CellPtr Cell = At(*Location);
			FVector WorldLocation = UGridFunctions::GridToWorldLocation(*Location);
			UWorld* World = GetWorld();
			if (World && Cell)
			{
				AMapPawn* NewPawn = Cast<AMapPawn>(World->SpawnActor(*MapPawnClasses[ClassIndex], &WorldLocation));
				if (NewPawn)
				{
					UE_LOG(LogNotice, Display, TEXT("Added new MapPawn with ID= %i : Owner= %i"), PawnID, OwnerID);

					NewPawn->SetOwnerID(OwnerID);
					NewPawn->SetPawnID(PawnID);
					Cell->SetOccupied(true);
					PawnCollection[OwnerID][PawnIndex] = std::make_unique<AMapPawn*>(NewPawn);
					NotifyConnectedPlayerOfNewPawn(OwnerID, PawnID);
					return PawnID;
				}
			}
		}
	}

	return 0;
}

bool AWorldGrid::FindPath(const FGridCoordinate & Start, const FGridCoordinate & End, TArray<FGridCoordinate>& OutPath)
{
	return UPathFinder::FindPath(this, Start, End, OutPath);
}

CellPtr AWorldGrid::At(const int X, const int Y) const
{
	CellPtr FoundCell = nullptr;

	if (LogicalGrid.size() > X && X >= 0)
	{
		if (LogicalGrid[X].size() > Y && Y >= 0)
		{
			FoundCell = LogicalGrid[X][Y];
		}
	}

	return FoundCell;
}

CellPtr AWorldGrid::At(const FGridCoordinate& Location) const
{
	return At(Location.X, Location.Y);
}

bool AWorldGrid::AddBlockingSpace(const FGridCoordinate& Location)
{
	CellPtr BlockingSpace = At(Location);
	if (BlockingSpace && !IsSpawnLocation(Location))
	{
		BlockingSpace->SetOccupied(true);
	}
	return false;
}

bool AWorldGrid::AddVisual(int ClassIndex, const FGridCoordinate & Location, bool makeMovableOverride)
{
	auto found = VisualGridRefrences.find(Location.toPair());
	if (found == VisualGridRefrences.end())
	{
		if (ClassIndex >= 0 && ClassIndex < InteractableClasses.Num() && InteractableClasses[ClassIndex])
		{
			FVector WorldLocation = UGridFunctions::GridToWorldLocation(Location);
			UWorld* World = GetWorld();
			if (World)
			{
				AInteractable* NewVisual = Cast<AInteractable>(World->SpawnActor(*InteractableClasses[ClassIndex], &WorldLocation));
				if (NewVisual)
				{
					if (!makeMovableOverride)
					{
						AddBlockingSpace(Location);
					}

					VisualGridRefrences[Location.toPair()] = NewVisual;
					return true;
				}
			}
		}
	}
	return false;
}

bool AWorldGrid::AddSpawnLocation(int ClassIndex, const FGridCoordinate & Location)
{
	CellPtr FoundCell = At(Location);
	bool bExists = IsSpawnLocation(Location);
	if (FoundCell && !FoundCell->bNonTraversable && !bExists)
	{
		int SpawnerClassNum = SpawnerClasses.Num();
		if (ClassIndex >= 0 && ClassIndex < SpawnerClassNum)
		{
			FVector WorldLocation = UGridFunctions::GridToWorldLocation(Location);
			UWorld* world = GetWorld();
			if (world)
			{
				ASpawner* NewSpawner = Cast<ASpawner>(world->SpawnActor(*SpawnerClasses[ClassIndex], &WorldLocation));

				if (NewSpawner)
				{
					SpawnLocations[Location.toPair()] = NewSpawner;
					return true;
				}
				else
				{
					UE_LOG(LogNotice, Error, TEXT("Failed to create spawn class"));
				}
			}
			else
			{
				UE_LOG(LogNotice, Error, TEXT("Could not pull world pointer while creating spawn location."));
			}
		}
		else
		{
			UE_LOG(LogNotice, Error, TEXT("Ignored spawn creation, could not find index matching type %s"), *FString::FromInt(ClassIndex));
		}
	}
	else
	{
		UE_LOG(LogNotice, Error, TEXT("Ignoring spawn creation, location already exists."));
	}

	return false;
}

bool AWorldGrid::AddSpawnLocations(int ClassIndex, const TArray<FGridCoordinate>& Locations)
{
	int index = 0;
	for (const auto& Coordinate : Locations)
	{
		if (!AddSpawnLocation(ClassIndex, Coordinate))
		{
			UE_LOG(LogNotice, Error, TEXT("Failed to add Spawn location from list at index= %i."), index);
			return false;
		}

		index++;
	}
	return true;
}

bool AWorldGrid::RemoveCharacter(int PawnID)
{
	int OwnerID;
	int PawnIndex;
	GetPawnIDParams(PawnID, OwnerID, PawnIndex);
	auto& found = PawnCollection[OwnerID][PawnIndex];
	if (found)
	{
		CellPtr CharacterLocation = At((*found)->GetActorLocation());
		if (CharacterLocation)
		{
			CharacterLocation->SetOccupied(false);
		}
		(*found)->Destroy();
		found = nullptr;

		return true;
	}

	return false;
}

bool AWorldGrid::RemoveVisual(const FGridCoordinate & Location)
{
	auto found = VisualGridRefrences.find(Location.toPair());
	if (found != VisualGridRefrences.end())
	{
		found->second->Destroy();
		found = VisualGridRefrences.end();
		VisualGridRefrences.erase(Location.toPair());
		return true;
	}
	return false;
}

bool AWorldGrid::RemoveBlockingSpace(const FGridCoordinate & Location)
{
	CellPtr BlockingSpace = At(Location);
	if (BlockingSpace)
	{
		BlockingSpace->SetOccupied(false);
		return true;
	}

	return false;
}

bool AWorldGrid::RemoveSpawnLocation(const FGridCoordinate & Location)
{
	bool bExists = IsSpawnLocation(Location);
	if (bExists)
	{
		SpawnLocations[Location.toPair()]->Destroy();
		SpawnLocations.erase(Location.toPair());
		return true;
	}

	return false;
}

void AWorldGrid::ClearCharacters()
{
	for (auto& character : PawnCollection)
	{
		for (auto& pawn : character.second)
		{
			if (pawn.second)
			{
				CellPtr CharacterLocation = At((*pawn.second)->GetActorLocation());
				if (CharacterLocation)
				{
					CharacterLocation->SetOccupied(false);
				}
				(*pawn.second)->Destroy();
				pawn.second = nullptr;
			}
		}
	}

	PawnCollection.clear();
}

void AWorldGrid::ClearSpawnLocations()
{
	for (const auto Location : SpawnLocations)
	{
		RemoveSpawnLocation(Location.first);
	}
}

int AWorldGrid::GetHostID() const
{
	return HostID;
}

AMapPawn* AWorldGrid::GetPawn(const int PlayerID, const int PawnID)
{
	int OwnerID;
	int PawnIndex;
	GetPawnIDParams(PawnID, OwnerID, PawnIndex);

	if (PlayerID == OwnerID)
	{
		auto& Selected = PawnCollection[OwnerID][PawnIndex];
		return (*Selected);
	}

	return nullptr;
}

std::shared_ptr<FGridCoordinate> AWorldGrid::GetOpenSpawnLocation()
{
	for (auto& spawn : SpawnLocations)
	{
		CellPtr SpawnLocation = At(spawn.first);
		if (SpawnLocation && !SpawnLocation->IsOcupied())
		{
			SpawnLocation->SetOccupied(true);
			return std::make_shared<FGridCoordinate>(spawn.first);
		}
	}

	return nullptr;
}

void AWorldGrid::GenerateGrid()
{
	for (int Row = 0; Row < Dimensions.X; Row++)
	{
		//make a new row for this column
		LogicalGrid.emplace_back();
		for (int Column = 0; Column < Dimensions.Y; Column++)
		{
			//Create new cell
			auto Current = MAKE_CELL(FGridCoordinate(Row, Column));
			LogicalGrid[Row].push_back(Current);
		}
	}

	for (const auto& XRow : LogicalGrid)
	{
		for (const auto& YRow : XRow)
		{
			int Row = YRow->Location.X;
			int Column = YRow->Location.Y;

			//Assign the nessesary neighbors
			auto Top    = At(Row, Column - 1);
			auto Bottom = At(Row, Column + 1);
			auto Left   = At(Row - 1, Column);
			auto Right  = At(Row + 1, Column);

			auto TopRight    = At(Row + 1, Column - 1);
			auto TopLeft     = At(Row - 1, Column - 1);
			auto BottomRight = At(Row + 1, Column + 1);
			auto BottomLeft  = At(Row - 1, Column + 1);

			YRow->GetNeighbor(Cell::CELL_TOP) = Top;
			YRow->GetNeighbor(Cell::CELL_BOTTOM) = Bottom;
			YRow->GetNeighbor(Cell::CELL_LEFT) = Left;
			YRow->GetNeighbor(Cell::CELL_RIGHT) = Right;

			YRow->GetNeighbor(Cell::CELL_TOPRIGHT) = TopRight;
			YRow->GetNeighbor(Cell::CELL_TOPLEFT) = TopLeft;
			YRow->GetNeighbor(Cell::CELL_BOTTOMRIGHT) = BottomRight;
			YRow->GetNeighbor(Cell::CELL_BOTTOMLEFT) = BottomLeft;
		}
	}
}

void AWorldGrid::SetupVisuals(const FGridCoordinate & GridDimensions)
{
	UStaticMeshComponent* PlaneMesh = Cast<UStaticMeshComponent>(RootComponent);
	if (PlaneMesh)
	{
		float Length_Meters, Width_Meters;
		float Length_Cm, Width_Cm;

		GetGridDimensions(Length_Meters, Width_Meters, UNITS::METERS);
		GetGridDimensions(Length_Cm, Width_Cm, UNITS::CENTIMETERS);

		PlaneMesh->SetRelativeLocation(FVector(-Length_Cm * GridDimensions.X * 0.5f, Width_Cm * GridDimensions.Y * 0.5f, 0.0f));
		PlaneMesh->SetWorldScale3D(FVector(Length_Meters * GridDimensions.X, Width_Meters * GridDimensions.Y, 1.0f));

		UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this);
		if (DynamicMaterial)
		{
			DynamicMaterial->SetVectorParameterValue(FName("Grid Dimensions"), FLinearColor((float)GridDimensions.X, (float)GridDimensions.Y, 0.0f, 0.0f));
			PlaneMesh->SetMaterial(0, DynamicMaterial);
		}
	}
}

bool AWorldGrid::SetPosition(const FGridCoordinate & Location, const FGridCoordinate & Destination)
{
	bool moved = false;
	if (Location != Destination)
	{
		CellPtr FoundLocation = At(Location);
		CellPtr FoundDestination = At(Destination);
		if (FoundLocation && FoundDestination && !FoundDestination->bNonTraversable)
		{
			FoundLocation->SetOccupied(false);
			FoundDestination->SetOccupied(true);
			moved = true;
		}

	}

	return moved;
}

int AWorldGrid::GenerateNewPawnIndex(const int OwnerID)
{
	int CharacterCount = PawnCollection[OwnerID].size();
	PawnCollection[OwnerID][CharacterCount] = nullptr;
	return CharacterCount;
}

void AWorldGrid::GetPawnIDParams(const int PawnID, int & OwnerID, int & PawnIndex) const
{
	PawnIndex = PawnID % 10;
	OwnerID = PawnID / 10;
}

void AWorldGrid::NotifyConnectedPlayerOfNewPawn(int PlayerID, int PawnID)
{
	auto ConnectedPlayer = PlayerCollection[PlayerID];
	if (ConnectedPlayer)
	{
		ConnectedPlayer->AddNewCharacter(PawnID);
	}
}

void AWorldGrid::OnGridDimensionRep()
{
	bInitialized = true;
	if (!HasAuthority())
	{
		SetupVisuals(Dimensions);
	}
}

//Cell Functions /////////////////////////////////////////////////

Cell::Cell()
{
	H_Cost = 0;
	G_Cost = 0;
	objectCount = 0;
	Parent = nullptr;
	bNonTraversable = false;
	Location = FGridCoordinate(-1, -1);

	for (auto& n : Neighbors)
		n = nullptr;
}

Cell::Cell(const FGridCoordinate& Location) :
	Cell()
{
	this->Location = Location;
}

int Cell::F_Cost() const
{
	return H_Cost + G_Cost;
}

bool Cell::operator<(const Cell & b)
{
	if (F_Cost() == b.F_Cost())
	{
		return H_Cost <= b.H_Cost;
	}

	return F_Cost() < b.F_Cost();
}

bool Cell::IsOcupied() const
{
	return (bool)objectCount;
}

void Cell::SetOccupied(bool value)
{
	if (value)
	{
		objectCount++;
	}
	else if (objectCount > 0)
	{
		objectCount--;
	}
}

CellPtr& Cell::operator[](const CELL_NEIGHBOR & Location)
{
	return GetNeighbor(Location);
}

CellPtr& Cell::GetNeighbor(const CELL_NEIGHBOR & Location)
{
	switch (Location)
	{
	case Cell::CELL_TOP:
		return Neighbors[0];
	case Cell::CELL_LEFT:
		return Neighbors[1];
	case Cell::CELL_RIGHT:
		return Neighbors[2];
	case Cell::CELL_BOTTOM:
		return Neighbors[3];
	case Cell::CELL_TOPLEFT:
		return Neighbors[4];
	case Cell::CELL_BOTTOMLEFT:
		return Neighbors[5];
	case Cell::CELL_TOPRIGHT:
		return Neighbors[6];
	case Cell::CELL_BOTTOMRIGHT:
		return Neighbors[7];
	}

	return Neighbors[0];
}

std::list<CellPtr> Cell::GetEmptyNeighbors()
{
	std::list<CellPtr> list;

	// 8 total possible neighbors
	for (int x = 0; x < 8; x++)
	{
		if (Neighbors[x] && !Neighbors[x]->bNonTraversable)
		{
			list.push_back(Neighbors[x]);
		}
	}

	return list;
}