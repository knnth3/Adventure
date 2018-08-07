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
	ActivePlayer = -1;
	OwnerID = 0;

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
		OwnerID = HostID;
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
		return CellLocation->isOcupied;
	}
	return false;
}

bool AWorldGrid::IsSpawnLocation(const FGridCoordinate & Location)const
{
	return (SpawnLocations.find(Location.toPair()) != SpawnLocations.end());
}

bool AWorldGrid::IsFreeRoamActive() const
{
	return (ActivePlayer == -1);
}

void AWorldGrid::BeginTurnBasedMechanics()
{
	ActivePlayer = 0;
}

void AWorldGrid::EndTurnBasedMechanics()
{
	ActivePlayer = -1;
}

bool AWorldGrid::MoveCharacter(const AConnectedPlayer* ConnectedPlayer, const FGridCoordinate& Destination, int PawnID)
{
	bool Success = false;
	if (ConnectedPlayer && ConnectedPlayer->PlayerState)
	{

		//Host requests a move
		if (ConnectedPlayer->PlayerState->PlayerId == OwnerID)
		{
			for (auto& Pawn : Characters)
			{
				if (Pawn.second.Pawn->GetPawnID() == PawnID)
				{
					Pawn.second.Pawn->SetDestination(Destination);
					Success = true;
				}
			}
		}
		else
		{
			//Client wants to make a move
			auto found = Characters.find(ConnectedPlayer->PlayerState->PlayerId);
			if (found != Characters.end())
			{
				if (IsFreeRoamActive())
				{
					found->second.Pawn->SetDestination(Destination);
					Success = true;
				}
				else if (found->second.Pawn->GetPawnID() == ActivePlayer)
				{
					found->second.Pawn->SetDestination(Destination);
					Success = true;
				}
			}
		}
	}

	if (Success)
	{
		UE_LOG(LogNotice, Warning, TEXT("Player move requested from %s: ID = %i"), *ConnectedPlayer->PlayerState->GetPlayerName(), ConnectedPlayer->PlayerState->PlayerId);
	}

	return Success;
}

bool AWorldGrid::RegisterPlayerController(AConnectedPlayer* ConnectedPlayer, int & CharacterID)
{
	bool success = false;
	if (ConnectedPlayer && ConnectedPlayer->PlayerState)
	{
		int PlayerID = ConnectedPlayer->PlayerState->PlayerId;
		if (PlayerID == OwnerID)
		{
			auto found = Characters.find(PlayerID);
			if (found != Characters.end())
			{
				found->second.Player = ConnectedPlayer;
			}

			CharacterID = -1;
			success = true;
		}
		else
		{
			auto found = Characters.find(PlayerID);
			if (found != Characters.end())
			{
				found->second.Player = ConnectedPlayer;
				CharacterID = found->second.Pawn->GetPawnID();
				success = true;
			}

		}
		UE_LOG(LogNotice, Warning, TEXT("Registering %s: ID = %i, Success = %i, Registered to PawnID = %i"), 
			*ConnectedPlayer->PlayerState->GetPlayerName(), ConnectedPlayer->PlayerState->PlayerId, success, CharacterID);
	}

	return success;
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

bool AWorldGrid::AddCharacter(int PlayerID, int ClassIndex)
{
	static int NextCharacterID = 0;
	if (Characters.find(PlayerID) == Characters.end())
	{
		if (ClassIndex >= 0 && ClassIndex < MapPawnClasses.Num() && MapPawnClasses[ClassIndex])
		{
			FGridCoordinate Location;
			if (GetOpenSpawnLocation(Location))
			{
				CellPtr Cell = At(Location);
				FVector WorldLocation = UGridFunctions::GridToWorldLocation(Location);
				UWorld* World = GetWorld();
				if (World && Cell)
				{
					AMapPawn* NewPawn = Cast<AMapPawn>(World->SpawnActor(*MapPawnClasses[ClassIndex], &WorldLocation));
					if (NewPawn)
					{
						UE_LOG(LogNotice, Display, TEXT("Added new MapPawn with ID= %i : Owner= %i"), NextCharacterID, PlayerID);

						NewPawn->SetOwnerID(PlayerID);
						NewPawn->SetPawnID(NextCharacterID++);

						Cell->isOcupied = true;

						//This is a bug for the host
						Characters[PlayerID].Pawn = NewPawn;
						return true;
					}
				}
			}
		}
	}

	return false;
}

bool AWorldGrid::AddBlockingSpace(const FGridCoordinate& Location)
{
	CellPtr BlockingSpace = At(Location);
	if (BlockingSpace && !IsSpawnLocation(Location))
	{
		BlockingSpace->isOcupied = true;
		BlockingSpace->isBlockingSpace = true;
	}
	return false;
}

bool AWorldGrid::AddVisual(int ClassIndex, const FGridCoordinate & Location)
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
	if (FoundCell && !FoundCell->isOcupied && !bExists)
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

bool AWorldGrid::RemoveCharacter(int PlayerID)
{
	auto found = Characters.find(PlayerID);
	if (Characters.find(PlayerID) != Characters.end() && found->second.Pawn)
	{
		CellPtr CharacterLocation = At(found->second.Pawn->GetActorLocation());
		if (CharacterLocation)
		{
			CharacterLocation->isOcupied = false;
			found->second.Pawn->Destroy();
			found = Characters.end();
			Characters.erase(PlayerID);
		}
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
		BlockingSpace->isOcupied = false;
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
	for (const auto& character : Characters)
	{
		if (character.second.Pawn)
		{
			CellPtr CharacterLocation = At(character.second.Pawn->GetActorLocation());
			if (CharacterLocation)
			{
				CharacterLocation->isOcupied = false;
				character.second.Pawn->Destroy();
			}
		}
	}

	Characters.clear();
}

void AWorldGrid::ClearBlockingSpaces()
{
	for (const auto& X : LogicalGrid)
	{
		for (const auto& cell : X)
		{
			if (cell->isBlockingSpace)
			{
				cell->isOcupied = false;
				cell->isBlockingSpace = false;
			}
		}
	}
}

void AWorldGrid::ClearSpawnLocations()
{
	for (const auto Location : SpawnLocations)
	{
		RemoveSpawnLocation(Location.first);
	}
}

bool AWorldGrid::GetOpenSpawnLocation(FGridCoordinate & Location)
{
	for (auto& spawn : SpawnLocations)
	{
		CellPtr SpawnLocation = At(spawn.first);
		if (SpawnLocation && !SpawnLocation->isOcupied)
		{
			SpawnLocation->isOcupied = true;
			Location = spawn.first;
			return true;
		}
	}

	return false;
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
		if (FoundLocation && FoundDestination && !FoundDestination->isOcupied)
		{
			FoundLocation->isOcupied = false;
			FoundDestination->isOcupied = true;
			moved = true;
		}

	}

	return moved;
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
	isBlockingSpace = false;
	isOcupied = false;
	H_Cost = 0;
	G_Cost = 0;
	Parent = nullptr;
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
		if (Neighbors[x] && !Neighbors[x]->isOcupied)
		{
			list.push_back(Neighbors[x]);
		}
	}

	return list;
}