// Fill out your copyright notice in the Description page of Project Settings.
#include "WorldGrid.h"

#include "Basics.h"
#include "Adventure.h"
#include "Interactable.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Runtime/Engine/Classes/Kismet/KismetMaterialLibrary.h"
#include "UObject/ConstructorHelpers.h"

#define MAKE_CELL(x) CellPtr(new Cell(x))
#define PRIORITY_QUEUE(T) std::priority_queue<std::shared_ptr<T>, std::vector<std::shared_ptr<T>>, shared_ptr_compare<T>>


// Sets default values
AWorldGrid::AWorldGrid()
{
	static ConstructorHelpers::FClassFinder<AInteractable> BP_Interactable(TEXT("/Game/Blueprints/Grid/BP_Interactable"));
	if (!BP_Interactable.Class)
	{
		UE_LOG(LogNotice, Error, TEXT("NO INTERACTABLE CLASS FOUND"));
	}

	InteractableClass = BP_Interactable.Class;

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bInitialized = false;

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

CellPtr AWorldGrid::At(FGridCoordinate Location) const
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
			bInitialized = true;
			GridDimensions = { Rows, Columns };
			SetUpGridVisual();
			SetUpGridLogical();
		}
	}
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

void AWorldGrid::SetSpawnLocations(const TArray<FGridCoordinate>& Locations)
{
	for (const auto& item : Locations)
	{
		CellPtr FoundCell = At(item);
		if (FoundCell)
		{
			SpawnLocations.push_back(FoundCell->Location);
		}
	}
}

bool AWorldGrid::GetOpenSpawnLocation(FGridCoordinate & GridLocation)
{
	for (const auto& location : SpawnLocations)
	{
		CellPtr FoundCell = At(location);
		if (FoundCell && !FoundCell->Ocupied)
		{
			GridLocation.X = location.X;
			GridLocation.Y = location.Y;
			FoundCell->Ocupied = true;
			return true;
		}
	}
	return false;
}

bool AWorldGrid::IsOccupied(FGridCoordinate Location) const
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

bool AWorldGrid::MovePawn(FGridCoordinate Location, FGridCoordinate Destination)
{
	bool moved = false;

	if (!(Location == Destination))
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

void AWorldGrid::RemoveActorFromPlay(FGridCoordinate Location)
{
	CellPtr FoundLocation = At(Location);
	if (FoundLocation)
	{
		FoundLocation->Ocupied = false;
	}
}

bool AWorldGrid::AddInteractible(int Type, FGridCoordinate Location)
{
	CellPtr FoundLocation = At(Location);
	if (FoundLocation && !FoundLocation->Ocupied)
	{
		FVector WorldLocation = UGridFunctions::GridToWorldLocation(Location);
		AInteractable* NewPawn = Cast<AInteractable>(GetWorld()->SpawnActor(*InteractableClass, &WorldLocation));
		if (NewPawn)
		{
			int MeshNum = InteractableMeshes.Num();

			if (MeshNum > Type && Type >= 0)
			{
				NewPawn->SetStaticMesh(InteractableMeshes[Type]);
			}

			FoundLocation->Ocupied = true;
			Interactables.Push(NewPawn);
			return true;
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

			//Link the top row if exists
			if (Row > 1)
			{
				//Top & Bottom
				auto Top = At(FGridCoordinate(Row - 1, Column));
				if (Top)
				{
					Current->GetNeighbor(Cell::CELL_TOP) = Top;
					Top->GetNeighbor(Cell::CELL_BOTTOM) = Current;
				}

				//Top Right & Bottom Left
				if (Column != GridDimensions.Columns)
				{
					auto TopRight = At(FGridCoordinate(Row - 1, Column + 1));
					if (TopRight)
					{
						Current->GetNeighbor(Cell::CELL_TOPRIGHT) = TopRight;
						TopRight->GetNeighbor(Cell::CELL_BOTTOMLEFT) = Current;
					}
				}
			}

			//Link the left column if exists
			if (Column > 1)
			{
				//Left & Right
				auto Left = At(FGridCoordinate(Row, Column - 1));
				if (Left)
				{
					Current->GetNeighbor(Cell::CELL_LEFT) = Left;
					Left->GetNeighbor(Cell::CELL_RIGHT) = Current;
				}

				//Top Left & Bottom Right
				if (Row > 1)
				{
					auto TopLeft = At(FGridCoordinate(Row - 1, Column - 1));
					if (TopLeft)
					{
						Current->GetNeighbor(Cell::CELL_TOPLEFT) = TopLeft;
						TopLeft->GetNeighbor(Cell::CELL_BOTTOMRIGHT) = Current;
					}
				}
			}
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

CellPtr Cell::operator[](const CELL_NEIGHBOR & Location)
{
	return GetNeighbor(Location);
}

CellPtr Cell::GetNeighbor(const CELL_NEIGHBOR & Location)
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
	default:
		return nullptr;
	}
}

std::list<CellPtr> Cell::GetEmptyNeighbors()
{
	std::list<CellPtr> list;

	// 8 total possible neighbors
	for (auto& n : Neighbors)
	{
		if (n && !n->Ocupied)
		{
			list.push_back(n);
		}
	}

	return list;
}
