// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.
#include "WorldGrid.h"
#include "./Character/MapPawn.h"
#include "Interactable.h"
#include "Spawner.h"
#include "PathFinder.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "RuntimeMeshComponent.h"

using namespace std;

AWorldGrid::AWorldGrid()
{
	bReplicates = true;
	bAlwaysRelevant = true;
	bReplicateMovement = false;
	bHasBeenConstructed = false;
	GeneratedAreaWidth = 1000.0f;
	GeneratedAreaHeightRange = 1000.0f;
	GeneratedAreaPlayAreaRandomIntensity = 1.0f;
	GeneratedAreaTesselation = 20;

	GridCellsMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Focus"));
	GridCellsMesh->SetIsReplicated(true);
	RootComponent = GridCellsMesh;

	RuntimeMesh = CreateDefaultSubobject<URuntimeMeshComponent>(TEXT("Surrounding"));
	RuntimeMesh->SetIsReplicated(true);
	RuntimeMesh->SetupAttachment(RootComponent);
}

void AWorldGrid::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const 
{
	DOREPLIFETIME(AWorldGrid, m_GridDimensions);
}

void AWorldGrid::BeginPlay()
{

}

void AWorldGrid::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ServerOnly_ResetGrid();
}

void AWorldGrid::ServerOnly_GenerateGrid(const FGridCoordinate & GridDimensions, std::vector<std::vector<int>> GridSheet)
{

	if (bHasBeenConstructed)
	{
		ServerOnly_ResetGrid();
	}

	int cellCount = 0;
	UWorld* World = GetWorld();
	if (World)
	{
		m_Grid.resize(GridDimensions.X);

		for (int x = 0; x < GridDimensions.X; x++)
		{
			m_Grid[x].resize(GridDimensions.Y, nullptr);

			for (int y = 0; y < GridDimensions.Y; y++)
			{
				FVector WorldLocation;
				AWorldGrid_Cell* newCell;
				if (CellClasses.Num() > 0)
				{
					WorldLocation = UGridFunctions::GridToWorldLocation(FGridCoordinate(x, y));
					newCell = Cast<AWorldGrid_Cell>(World->SpawnActor(*CellClasses[0], &WorldLocation));
				}
				else // Default to base class when no derivatives exist
				{
					FRotator startRotation(0.0f);
					WorldLocation = UGridFunctions::GridToWorldLocation(FGridCoordinate(x, y));
					newCell = Cast<AWorldGrid_Cell>(World->SpawnActor<AWorldGrid_Cell>(WorldLocation, startRotation));
				}

				if (newCell)
				{
					// Create Visual Representation
					GridCellsMesh->AddInstanceWorldSpace(FTransform(FRotator(0.0f), WorldLocation, FVector(1.525f, 1.525f, 1.0f)));
					newCell->Initialize(FGridCoordinate(x, y));
					m_Grid[x][y] = newCell;
					ServerOnly_LinkCell(newCell);
					cellCount++;
				}
			}
		}
	}

	if (cellCount == GridDimensions.X * GridDimensions.Y)
	{
		m_GridDimensions = GridDimensions;
		OnRep_HasBeenConstructed();
		UE_LOG(LogNotice, Warning, TEXT("<Grid Generation>: Finished generating grid"));
	}
	else
	{
		UE_LOG(LogNotice, Warning, TEXT("<Grid Generation>: Failed to generate grid"));
	}
}

void AWorldGrid::ServerOnly_GenerateGrid(const FGridCoordinate & Dimensions)
{
	ServerOnly_GenerateGrid(Dimensions, vector<vector<int>>());
}

void AWorldGrid::ServerOnly_ResetGrid()
{
	m_GridDimensions = FGridCoordinate(0, 0);
	for (auto& obj : m_Visuals)
	{
		obj->Destroy();
	}
	for (auto& obj : m_MapPawns)
	{
		obj->Destroy();
	}
	for (auto& obj : m_Spawns)
	{
		obj->Destroy();
	}
	for (auto& row : m_Grid)
	{
		for (auto& cell : row)
		{
			cell->Destroy();
		}
	}

	m_Visuals.clear();
	m_MapPawns.clear();
	m_Spawns.clear();
	m_Grid.clear();
	GridCellsMesh->ClearInstances();
}

bool AWorldGrid::ServerOnly_AddVisual(int ClassIndex, const FGridCoordinate & Location)
{
	if (ContainsCoordinate(Location.X, Location.Y))
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
					int newIndex = FindEmptyIndex(m_Visuals);
					m_Visuals[newIndex] = NewVisual;
					NewVisual->ServerOnly_SetClassIndex(ClassIndex);
					NewVisual->ServerOnly_SetObjectID(newIndex);
					return true;
				}
			}
		}
	}
	return false;
}

bool AWorldGrid::ServerOnly_RemoveVisual(const FGridCoordinate& Location)
{
	if (ContainsCoordinate(Location.X, Location.Y))
	{
		int index = m_Grid[Location.X][Location.Y]->RemoveVisual();
		if (index != -1)
		{
			m_Visuals[index]->Destroy();
			m_Visuals[index] = nullptr;
			return true;
		}
	}

	return false;
}

bool AWorldGrid::ServerOnly_AddBlockingObject(int ClassIndex, const FGridCoordinate & Location)
{
	if (ContainsCoordinate(Location.X, Location.Y))
	{
		if (ClassIndex >= 0 && ClassIndex < InteractableClasses.Num() && InteractableClasses[ClassIndex])
		{
			FVector WorldLocation = UGridFunctions::GridToWorldLocation(Location);
			UWorld* World = GetWorld();
			if (World)
			{
				AInteractable* NewBlockingObject = Cast<AInteractable>(World->SpawnActor(*InteractableClasses[ClassIndex], &WorldLocation));
				if (NewBlockingObject)
				{
					int newIndex = FindEmptyIndex(m_Visuals);
					m_Visuals[newIndex] = NewBlockingObject;
					NewBlockingObject->ServerOnly_SetClassIndex(ClassIndex);
					NewBlockingObject->ServerOnly_SetObjectID(newIndex);
					return true;
				}
			}
		}
	}
	return false;
}

bool AWorldGrid::ServerOnly_RemoveBlockingObject(const FGridCoordinate& Location)
{
	if (ContainsCoordinate(Location.X, Location.Y))
	{
		int index = m_Grid[Location.X][Location.Y]->RemoveBlockingObject();
		if (index != -1)
		{
			m_Visuals[index]->Destroy();
			m_Visuals[index] = nullptr;
			return true;
		}
	}

	return false;
}

bool AWorldGrid::ServerOnly_AddSpawnLocation(int ClassIndex, const FGridCoordinate & Location)
{
	if (ContainsCoordinate(Location.X, Location.Y))
	{
		if (ClassIndex >= 0 && ClassIndex < SpawnerClasses.Num() && SpawnerClasses[ClassIndex])
		{
			FVector WorldLocation = UGridFunctions::GridToWorldLocation(Location);
			UWorld* World = GetWorld();
			if (World)
			{
				ASpawner* NewSpawner = Cast<ASpawner>(World->SpawnActor(*SpawnerClasses[ClassIndex], &WorldLocation));
				if (NewSpawner)
				{
					int newIndex = FindEmptyIndex(m_Spawns);
					m_Spawns[newIndex] = NewSpawner;
					m_SpawnLocations.push_back(Location);
					return true;
				}
			}
		}
	}
	return false;
}

bool AWorldGrid::ServerOnly_RemoveSpawnLocation(const FGridCoordinate& Location)
{
	if (ContainsCoordinate(Location.X, Location.Y))
	{
		int index = m_Grid[Location.X][Location.Y]->RemoveSpawner();
		if (index != -1)
		{
			//Remove spawnLocation from list
			FGridCoordinate temp = m_SpawnLocations.back();
			for (auto& loc : m_SpawnLocations)
			{
				if (loc == Location)
				{
					loc = temp;
					m_SpawnLocations.pop_back();
					break;
				}
			}

			m_Spawns[index]->Destroy();
			m_Spawns[index] = nullptr;
			return true;
		}
	}

	return false;
}

bool AWorldGrid::ServerOnly_AddPawn(int ClassIndex, const FGridCoordinate & Location, int OwningPlayerID)
{
	if (ContainsCoordinate(Location.X, Location.Y))
	{
		if (ClassIndex >= 0 && ClassIndex < MapPawnClasses.Num() && MapPawnClasses[ClassIndex])
		{
			FVector WorldLocation = UGridFunctions::GridToWorldLocation(Location);
			UWorld* World = GetWorld();
			if (World)
			{
				FTransform pawnTransform(FRotator(0, 180, 0), WorldLocation);
				AMapPawn* NewPawn = Cast<AMapPawn>(World->SpawnActor(*MapPawnClasses[ClassIndex], &pawnTransform));
				if (NewPawn)
				{
					m_PlayerPawnCount[OwningPlayerID]++;
					int pawnID = FindEmptyIndex(m_MapPawns);
					m_MapPawns[pawnID] = NewPawn;
					NewPawn->ServerOnly_SetPawnID(pawnID);
					NewPawn->ServerOnly_SetClassIndex(ClassIndex);
					return true;
				}
			}
		}
	}
	return false; 
}

bool AWorldGrid::ServerOnly_RemovePawn(const FGridCoordinate& Location, int pawnID)
{
	if (ContainsCoordinate(Location.X, Location.Y))
	{
		int index = m_Grid[Location.X][Location.Y]->RemovePawn(pawnID);
		if (index != -1)
		{
			AMapPawn* replacement = nullptr;
			if (m_MapPawns.size() > 1)
			{
				replacement = m_MapPawns.back();
				replacement->ServerOnly_SetPawnID(pawnID);
			}

			m_MapPawns[index]->Destroy();
			m_MapPawns[index] = replacement;
			m_MapPawns.pop_back();
			return true;
		}
	}

	return false;
}

AMapPawn * AWorldGrid::ServerOnly_GetPawn(int pawnID)
{
	for (auto& pawn : m_MapPawns)
	{
		if (pawn)
		{
			if (pawn->GetPawnID() == pawnID)
			{
				return pawn;
			}
		}
	}
	return nullptr;
}

TArray<AMapPawn*> AWorldGrid::ServerOnly_GetAllPawns(int pawnID)
{
	TArray<AMapPawn*> arr;
	arr.Append(&m_MapPawns[0], m_MapPawns.size());
	return arr;
}

bool AWorldGrid::ServerOnly_GetPath(const FGridCoordinate & Location, const FGridCoordinate & Destination, TArray<FGridCoordinate>& OutPath)
{
	bool startExists = ContainsCoordinate(Location.X, Location.Y);
	bool endExists = ContainsCoordinate(Destination.X, Destination.Y);

	if (startExists && endExists)
	{
		return UPathFinder::FindPath(m_Grid[Location.X][Location.Y], m_Grid[Destination.X][Destination.Y], OutPath);
	}

	return false;
}

FGridCoordinate AWorldGrid::ServerOnly_GetOpenSpawnLocation()const
{
	for (const auto& loc : m_SpawnLocations)
	{
		auto cell = m_Grid[loc.X][loc.Y];
		if (!cell->ContainsPawn())
		{
			return loc;
		}
	}
	return FGridCoordinate();
}

void AWorldGrid::GenerateEnvironment(const FGridCoordinate& GridDimensions)
{
	// Generate the visual for each cell
	for (int x = 0; x < GridDimensions.X; x++)
	{
		for (int y = 0; y < GridDimensions.Y; y++)
		{
			FGridCoordinate cellCoordinate(x, y);
			GridCellsMesh->AddInstanceWorldSpace(FTransform(FRotator(0.0f), UGridFunctions::GridToWorldLocation(cellCoordinate), FVector(1.525f, 1.525f, 1.0f)));
		}
	}

	// Generate the visual for the backdrop
	GenerateBackdrop(GridDimensions);
}

bool AWorldGrid::ContainsCoordinate(int x, int y)
{
	if (m_Grid.size() > x && x >= 0)
	{
		if (m_Grid[x].size() > y && y >= 0)
		{
			return m_Grid[x][y];
		}
	}

	return false;
}

void AWorldGrid::ServerOnly_LinkCell(AWorldGrid_Cell * NewCell)
{
	FGridCoordinate center = NewCell->Location;
	int top = center.Y - 1;
	int left = center.X - 1;
	int bottom = center.Y + 1;

	bool topExists = ContainsCoordinate(center.X, top);
	bool leftExists = ContainsCoordinate(left, center.Y);
	bool bottomLeftExists = ContainsCoordinate(left, bottom);
	bool topLeftExists = ContainsCoordinate(left, top);

	if (bottomLeftExists)
	{
		m_Grid[left][bottom]->Neigbor(AWorldGrid_Cell::NEIGHBOR::TOPRIGHT) = NewCell;
		NewCell->Neigbor(AWorldGrid_Cell::NEIGHBOR::BOTTOMLEFT) = m_Grid[left][bottom];
	}
	if (topLeftExists)
	{
		m_Grid[left][top]->Neigbor(AWorldGrid_Cell::NEIGHBOR::BOTTOMRIGHT) = NewCell;
		NewCell->Neigbor(AWorldGrid_Cell::NEIGHBOR::TOPLEFT) = m_Grid[left][top];
	}
	if (leftExists)
	{
		m_Grid[left][center.Y]->Neigbor(AWorldGrid_Cell::NEIGHBOR::RIGHT) = NewCell;
		NewCell->Neigbor(AWorldGrid_Cell::NEIGHBOR::LEFT) = m_Grid[left][center.Y];
	} 
	if (topExists)
	{
		m_Grid[center.X][top]->Neigbor(AWorldGrid_Cell::NEIGHBOR::BOTTOM) = NewCell;
		NewCell->Neigbor(AWorldGrid_Cell::NEIGHBOR::TOP) = m_Grid[center.X][top];
	}
}

void AWorldGrid::GenerateBackdrop(const FGridCoordinate& GridDimensions)
{
	TArray<FRuntimeMeshVertexSimple> Vertices;
	TArray<int32> Triangles;

	float width = GeneratedAreaWidth;
	float height = GeneratedAreaWidth;
	float gridWidth = Conversions::Feet::ToCentimeters(GridDimensions.Y * 5.0f);
	float gridHeight = Conversions::Feet::ToCentimeters(GridDimensions.X * 5.0f);
	float totalWidth = width + gridWidth;
	float totalHeight = height + gridHeight;

	MeshLibrary::GenerateGrid(Vertices, Triangles, GeneratedAreaTesselation, GeneratedAreaTesselation, 
		2.0f * width + gridWidth, 2.0f * height + gridHeight, -width - gridWidth, -height); // To

	FVector center(gridWidth / -2.0f, gridHeight / 2.0f, 0.0f);
	for (auto& v : Vertices)
	{
		FVector length = center - v.Position;
		float posHeight = GetGeneratedHeightValue(FVector2D(v.Position.X / totalWidth, v.Position.Y / totalHeight));
		float vecDist = FMath::Sqrt(length.X * length.X + length.Y * length.Y);
		float acceptableRadius = (gridWidth > gridHeight) ? gridWidth : gridHeight;
		acceptableRadius = sqrt(acceptableRadius * acceptableRadius + acceptableRadius * acceptableRadius + acceptableRadius * acceptableRadius);
		float finalHeight = posHeight;

		if (vecDist <= acceptableRadius)
		{
			finalHeight = -0.0001f;
		}

		v.Position.Z = finalHeight * GeneratedAreaHeightRange;
	}

	// Create the mesh section
	RuntimeMesh->CreateMeshSection(0, Vertices, Triangles);
	RuntimeMesh->SetMaterial(0, GeneratedAreaMaterial);
}

void AWorldGrid::OnRep_HasBeenConstructed()
{
	GenerateEnvironment(m_GridDimensions);
	bHasBeenConstructed = true;
}
