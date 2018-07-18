// Fill out your copyright notice in the Description page of Project Settings.
#include "WorldGrid.h"

#include "Adventure.h"
#include "Interactable.h"
#include "Spawner.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Runtime/Engine/Classes/Kismet/KismetMaterialLibrary.h"
#include "UObject/ConstructorHelpers.h"
#include "PathFinder.h"
#include "Character/MapPawn.h"
#include "Kismet/GameplayStatics.h"
#include "GameModes/GM_Multiplayer.h"


// Sets default values
AWorldGrid::AWorldGrid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bInitialized = false;
	HostPlayerID = 0;

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

void AWorldGrid::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWorldGrid, Scale);
	DOREPLIFETIME(AWorldGrid, GridDimensions);
}

CellPtr AWorldGrid::At(const FGridCoordinate& Location) const
{
	CellPtr FoundCell = nullptr;

	if (LogicalGrid.size() > Location.X && Location.X >= 0)
	{
		if (LogicalGrid[Location.X].size() > Location.Y && Location.Y >= 0)
		{
			FoundCell = LogicalGrid[Location.X][Location.Y];
		}
	}

	return FoundCell;
}

void AWorldGrid::Initialize(int Rows, int Columns)
{
	if (!bInitialized)
	{
		if (HasAuthority())
		{
			APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
			if (PlayerController)
			{
				HostPlayerID = PlayerController->PlayerState->PlayerId;
				bInitialized = true;
				GridDimensions = { Rows, Columns };
				SetUpGridVisual();
				SetUpGridLogical();

			}
			else
			{
				UE_LOG(LogNotice, Error, TEXT("Unable to grab the servers playerID. World Grid Init failed!"));
			}
		}
	}
}

bool AWorldGrid::SetSpawnLocation(int type, const FGridCoordinate & Location)
{
	CellPtr FoundCell = At(Location);
	bool bExists = CheckIfSpawnLocation(Location);
	if (FoundCell && !FoundCell->Ocupied && !bExists)
	{
		int SpawnerClassNum = SpawnerClasses.Num();
		if (type >= 0 && type < SpawnerClassNum)
		{
			FVector WorldLocation = UGridFunctions::GridToWorldLocation(Location);
			UWorld* world = GetWorld();
			if (world)
			{
				ASpawner* NewPawn = Cast<ASpawner>(world->SpawnActor(*SpawnerClasses[type], &WorldLocation));

				if (NewPawn)
				{
					VisualGridRefrences[Location.toPair()] = NewPawn;
					SpawnLocations[Location.toPair()] = Location;
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
			UE_LOG(LogNotice, Error, TEXT("Ignored spawn creation, could not find index matching type %s"), *FString::FromInt(type));
		}
	}
	else
	{
		UE_LOG(LogNotice, Error, TEXT("Ignoring spawn creation, location already exists."));
	}

	return false;
}

bool AWorldGrid::RemoveSpawnLocation(const FGridCoordinate & Location)
{
	if (CheckIfSpawnLocation(Location))
	{
		UWorld* world = GetWorld();
		if (world)
		{
			SpawnLocations.erase(Location.toPair());
			world->DestroyActor(VisualGridRefrences[Location.toPair()]);
			VisualGridRefrences.erase(Location.toPair());

			return true;
		}
	}

	return false;
}

bool AWorldGrid::CheckIfSpawnLocation(const FGridCoordinate & Location)
{
	return (SpawnLocations.find(Location.toPair()) != SpawnLocations.end());
}

// Called when the game starts or when spawned
void AWorldGrid::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AWorldGrid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//GetStringOf(Role);
	FString role = FString::FromInt(GridDimensions.Rows) + ", " + FString::FromInt(GridDimensions.Columns);
	DrawDebugString(GetWorld(), FVector(-100, 100, 0), role, this, FColor::White, DeltaTime);
}

FGridCoordinate AWorldGrid::GetSize() const
{
	return FGridCoordinate(GridDimensions.Rows, GridDimensions.Columns);
}

bool AWorldGrid::GetPath(const FGridCoordinate & Start, const FGridCoordinate & End, TArray<FGridCoordinate>& OutPath)
{
	return UPathFinder::FindPath(this, Start, End, OutPath);
}

bool AWorldGrid::IsPlayersTurn(const int PlayerID)
{
	AGM_Multiplayer* Gamemode = Cast<AGM_Multiplayer>(GetWorld()->GetAuthGameMode());
	if (Gamemode)
	{
		return (HostPlayerID == PlayerID) || Gamemode->IsPlayersTurn(PlayerID) || (PlayerID == -1);
	}
	else
	{
		return true;
	}
}

bool AWorldGrid::SetPosition(const FGridCoordinate & Location, const FGridCoordinate & Destination)
{
	bool moved = false;

	if (Location != Destination)
	{
		CellPtr FoundLocation = At(Location);
		CellPtr FoundDestination = At(Destination);
		if (FoundLocation && FoundDestination && !FoundDestination->Ocupied)
		{
			FoundLocation->Ocupied = false;
			FoundDestination->Ocupied = true;
			moved = true;
		}

	}

	return moved;
}

void AWorldGrid::SetSpawnLocations(const TArray<FGridCoordinate>& Locations)
{
	for (const auto& item : Locations)
	{
		SetSpawnLocation(0, item);
	}
}

bool AWorldGrid::SpawnMapPawn(const int PlayerID)
{
	//For right now, only allow one Map pawn per ID
	if (MapPawns.find(PlayerID) == MapPawns.end())
	{
		for (const auto& location : SpawnLocations)
		{
			CellPtr FoundCell = At(location.second);
			if (FoundCell && !FoundCell->Ocupied)
			{
				if (MapPawnClasses.Num() && MapPawnClasses[0])
				{
					FVector WorldLocation = UGridFunctions::GridToWorldLocation(location.second);
					UWorld* World = GetWorld();
					if (World)
					{
						AMapPawn* NewPawn = Cast<AMapPawn>(World->SpawnActor(*MapPawnClasses[0], &WorldLocation));
						if (NewPawn)
						{
							NewPawn->SetOwnerID(PlayerID);
							NewPawn->SetPawnID(MapPawns.size());
							MapPawns[PlayerID] = NewPawn;

							FoundCell->Ocupied = true;
							UE_LOG(LogNotice, Display, TEXT("Added new MapPawn with ID= %i : Owner= %i"), NewPawn->GetPawnID(), NewPawn->GetOwnerID());
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}

bool AWorldGrid::IsOccupied(const FGridCoordinate& Location) const
{
	CellPtr FoundCell = At(Location);

	if (FoundCell)
	{
		FString occupied = FoundCell->Ocupied ? "True" : "False";
		UE_LOG(LogNotice, Display, TEXT("Is Occupied at (%d, %d): %s"), Location.X, Location.Y, *occupied);
		return FoundCell->Ocupied;
	}

	UE_LOG(LogNotice, Warning, TEXT("Not a valid move spot at (%d, %d)"), Location.X, Location.Y);
	return true;
}

void AWorldGrid::MovePawn(const int PlayerID, const FGridCoordinate& Destination, const int PawnID)
{
	if (PlayerID == HostPlayerID && PawnID != -1)
	{
		int count = 0;
		for (auto& Pawn : MapPawns)
		{
			if (count == PawnID)
			{
				Pawn.second->SetDestination(Destination);
			}

			count++;
		}
	}
	else
	{
		auto found = MapPawns.find(PlayerID);

		if (found != MapPawns.end())
		{
			found->second->SetDestination(Destination);
		}
	}
}

void AWorldGrid::RemoveActorFromPlay(const FGridCoordinate& Location)
{
	CellPtr FoundLocation = At(Location);
	if (FoundLocation)
	{
		FoundLocation->Ocupied = false;
	}
}

bool AWorldGrid::AddInteractable(int Type, const FGridCoordinate& Location)
{
	CellPtr FoundCell = At(Location);
	bool bExists = CheckIfSpawnLocation(Location);
	if (FoundCell && !FoundCell->Ocupied && !bExists)
	{
		int InteractableClassNum = InteractableClasses.Num();
		if (Type >= 0 && Type < InteractableClassNum)
		{
			FVector WorldLocation = UGridFunctions::GridToWorldLocation(Location);
			UWorld* World = GetWorld();
			if (World)
			{
				AInteractable* NewPawn = Cast<AInteractable>(World->SpawnActor(*InteractableClasses[Type], &WorldLocation));

				if (NewPawn)
				{
					VisualGridRefrences[Location.toPair()] = NewPawn;
					FoundCell->Ocupied = true;
					return true;
				}
				else
				{
					UE_LOG(LogNotice, Error, TEXT("Failed to create Interactable class"));
				}
			}
			else
			{
				UE_LOG(LogNotice, Error, TEXT("Could not pull world pointer while creating Interactable location."));
			}
		}
		else
		{
			UE_LOG(LogNotice, Error, TEXT("Ignored Interactable creation, could not find index matching type %s"), *FString::FromInt(Type));
		}
	}
	else
	{
		UE_LOG(LogNotice, Error, TEXT("Ignoring Interactable creation, location already exists."));
	}

	return false;
}

bool AWorldGrid::RemoveInteractable(const FGridCoordinate & Location)
{
	CellPtr FoundLocation = At(Location);
	if (FoundLocation)
	{
		UWorld* world = GetWorld();
		if (world)
		{
			if (world->DestroyActor(VisualGridRefrences[Location.toPair()]))
			{
				VisualGridRefrences.erase(Location.toPair());
				FoundLocation->Ocupied = false;

				return true;
			}
		}
	}

	return false;
}

void AWorldGrid::OnScale_Rep()
{
	PlaneVisual->SetWorldScale3D(Scale);
}

void AWorldGrid::OnDimensions_Rep()
{
	SetUpGridVisual();
}

void AWorldGrid::SetUpGridVisual()
{
	UStaticMeshComponent* PlaneMesh = Cast<UStaticMeshComponent>(RootComponent);
	if (PlaneMesh)
	{

		float Length_Meters, Width_Meters;
		float Length_Cm, Width_Cm;

		GetGridDimensions(Length_Meters, Width_Meters, UNITS::METERS);
		GetGridDimensions(Length_Cm, Width_Cm, UNITS::CENTIMETERS);

		PlaneMesh->SetRelativeLocation(FVector(-Length_Cm * GridDimensions.Rows * 0.5f, Width_Cm * GridDimensions.Columns * 0.5f, 0.0f));
		PlaneMesh->SetWorldScale3D(FVector(Length_Meters * GridDimensions.Rows, Width_Meters * GridDimensions.Columns, 1.0f));

		UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(Material, this);
		if (DynamicMaterial)
		{
			DynamicMaterial->SetVectorParameterValue(FName("Grid Dimensions"), FLinearColor((float)GridDimensions.Rows, (float)GridDimensions.Columns, 0.0f, 0.0f));
			PlaneMesh->SetMaterial(0, DynamicMaterial);
		}
	}
}

void AWorldGrid::SetUpGridLogical()
{
	for (int Row = 0; Row < GridDimensions.Rows; Row++)
	{
		//make a new row for this column
		LogicalGrid.emplace_back();
		for (int Column = 0; Column < GridDimensions.Columns; Column++)
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
			auto Top         = At(FGridCoordinate(Row, Column - 1));
			auto Bottom      = At(FGridCoordinate(Row, Column + 1));
			auto Left        = At(FGridCoordinate(Row - 1, Column));
			auto Right       = At(FGridCoordinate(Row + 1, Column));

			auto TopRight    = At(FGridCoordinate(Row + 1, Column - 1));
			auto TopLeft     = At(FGridCoordinate(Row - 1, Column - 1));
			auto BottomRight = At(FGridCoordinate(Row + 1, Column + 1));
			auto BottomLeft  = At(FGridCoordinate(Row - 1, Column + 1));

			YRow->GetNeighbor(Cell::CELL_TOP)    = Top;
			YRow->GetNeighbor(Cell::CELL_BOTTOM) = Bottom;
			YRow->GetNeighbor(Cell::CELL_LEFT)   = Left;
			YRow->GetNeighbor(Cell::CELL_RIGHT)  = Right;

			YRow->GetNeighbor(Cell::CELL_TOPRIGHT)    = TopRight;
			YRow->GetNeighbor(Cell::CELL_TOPLEFT)     = TopLeft;
			YRow->GetNeighbor(Cell::CELL_BOTTOMRIGHT) = BottomRight;
			YRow->GetNeighbor(Cell::CELL_BOTTOMLEFT)  = BottomLeft;
		}
	}

	for (int Row = 0; Row < GridDimensions.Rows; Row++)
	{
		//make a new row for this column
		LogicalGrid.emplace_back();
		for (int Column = 0; Column < GridDimensions.Columns; Column++)
		{

		}
	}
}

Cell::Cell()
{
	Ocupied = false;
	H_Cost = 0;
	G_Cost = 0;
	Parent = nullptr;
	Location = FGridCoordinate(-1, -1);

	for (auto& n : Neighbors)
		n = nullptr;
}

Cell::Cell(FGridCoordinate Location):
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
		if (Neighbors[x] && !Neighbors[x]->Ocupied)
		{
			list.push_back(Neighbors[x]);
		}
	}

	return list;
}
