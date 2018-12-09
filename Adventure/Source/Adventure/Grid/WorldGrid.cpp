// By: Eric Marquez. All information and code provided is free to use and can be used comercially.
// Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.
#include "WorldGrid.h"
#include "./Character/MapPawn.h"
#include "Interactable.h"
#include "Grid/GridEntity.h"
#include "Spawner.h"
#include "PathFinder.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"

using namespace std;

AWorldGrid::AWorldGrid()
{
	bReplicates = true;
	bShowCollisions = false;
	bAlwaysRelevant = true;
	bReplicateMovement = false;
	bGenerateBackDrop = false;
	m_bMapIsLoaded = false;
	GeneratedAreaWidth = 1000.0f;
	GeneratedAreaHeightRange = 1000.0f;
	GeneratedAreaPlayAreaRandomIntensity = 1.0f;
	GeneratedAreaTesselation = 20;
	m_GridDimensions = FGridCoordinate(100, 100);
}

void AWorldGrid::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const 
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWorldGrid, m_MapName);
}

bool AWorldGrid::ServerOnly_SaveMap()
{
	UMapSaveFile* SaveGameInstance = Cast<UMapSaveFile>(UGameplayStatics::CreateSaveGameObject(UMapSaveFile::StaticClass()));
	SaveGameInstance->MapName = m_MapName;
	SaveGameInstance->MapSize = m_GridDimensions;
	SaveGameInstance->HeightMap.SetNum(m_GridDimensions.X * m_GridDimensions.Y);

	// Load map and save here
	for (const auto& index : m_UsedCellIndices)
	{
		auto InstancedMesh = GetCellInstanceMesh(index);
		if (InstancedMesh)
		{
			int count = InstancedMesh->GetInstanceCount();
			for (int x = 0; x < count; x++)
			{
				FTransform CellTransform;
				if (InstancedMesh->GetInstanceTransform(x, CellTransform, true))
				{
					FGridCoordinate GridCoordinates = UGridFunctions::WorldToGridLocation(CellTransform.GetTranslation());
					int loc = (GridCoordinates.Y * m_GridDimensions.X) + GridCoordinates.X;
					int height = (uint8)(CellTransform.GetScale3D().Z / CELL_STEP);

					SaveGameInstance->HeightMap[loc] = height;
				}
			}
		}
	}

	FString path = FString::Printf(TEXT("%sMaps/%s.map"), *FPaths::ProjectUserDir(), *SaveGameInstance->MapName);

	UE_LOG(LogNotice, Warning, TEXT("Map saved at: %s"), *path);

	UBasicFunctions::SaveFile(SaveGameInstance, path);
	return true;
}

bool AWorldGrid::ServerOnly_LoadGrid(const FString& MapName)
{
	TArray<FSAVE_OBJECT> GridSheet;
	FString path = FString::Printf(TEXT("%sMaps/%s.map"), *FPaths::ProjectUserDir(), *MapName);

	if (FPaths::FileExists(path))
	{
		// Map was loaded successfully, Tell client to load the map as well
		m_MapName = MapName;
		OnRep_BuildMap();
	}

	return false;
}

bool AWorldGrid::ServerOnly_GenerateGrid(const FString& MapName, const FGridCoordinate& MapSize)
{
	m_MapName = MapName;
	return GeneratePlayArea(MapSize, nullptr);
}

void AWorldGrid::ServerOnly_ResetGrid()
{
	m_GridDimensions = FGridCoordinate(0,0,0);
	for (const auto& index : m_UsedCellIndices)
	{
		auto InstancedMesh = GetCellInstanceMesh(index);
		if (InstancedMesh)
		{
			InstancedMesh->ClearInstances();
		}
	}

	m_UsedCellIndices.clear();
	m_PlayerPawnCount.clear();
	m_bMapIsLoaded = false;
}

bool AWorldGrid::ServerOnly_AddBlockingObject(int ClassIndex, const FGridCoordinate & Location)
{
	//if (ContainsCoordinate(Location.X, Location.Y) && !m_Grid[Location.X][Location.Y]->IsOcupied())
	//{
	//	if (ClassIndex >= 0 && ClassIndex < InteractableClasses.Num() && InteractableClasses[ClassIndex])
	//	{
	//		FVector WorldLocation = UGridFunctions::GridToWorldLocation(Location);
	//		UWorld* World = GetWorld();
	//		if (World)
	//		{
	//			AInteractable* NewBlockingObject = Cast<AInteractable>(World->SpawnActor(*InteractableClasses[ClassIndex], &WorldLocation));
	//			if (NewBlockingObject)
	//			{
	//				NewBlockingObject->ServerOnly_SetClassIndex(ClassIndex);

	//				return true;
	//			}
	//		}
	//	}
	//}
	return false;
}

bool AWorldGrid::ServerOnly_RemoveBlockingObject(const FGridCoordinate& Location)
{
	//if (ContainsCoordinate(Location.X, Location.Y))
	//{
	//	if (m_Grid[Location.X][Location.Y]->GetObjectType() == GRID_OBJECT_TYPE::INTERACTABLE)
	//	{
	//		AActor* object = m_Grid[Location.X][Location.Y]->RemoveObject();
	//		if (object)
	//		{
	//			object->Destroy();
	//			return true;
	//		}
	//	}
	//}

	return false;
}

bool AWorldGrid::ServerOnly_AddSpawnLocation(int ClassIndex, const FGridCoordinate & Location)
{
	//if (ContainsCoordinate(Location.X, Location.Y) && !m_Grid[Location.X][Location.Y]->IsOcupied())
	//{
	//	if (ClassIndex >= 0 && ClassIndex < SpawnerClasses.Num() && SpawnerClasses[ClassIndex])
	//	{
	//		FVector WorldLocation = UGridFunctions::GridToWorldLocation(Location);
	//		UWorld* World = GetWorld();
	//		if (World)
	//		{
	//			ASpawner* NewSpawner = Cast<ASpawner>(World->SpawnActor(*SpawnerClasses[ClassIndex], &WorldLocation));
	//			if (NewSpawner)
	//			{
	//				m_SpawnLocations.push_back(Location);
	//				return true;
	//			}
	//		}
	//	}
	//}
	return false;
}

bool AWorldGrid::ServerOnly_RemoveSpawnLocation(const FGridCoordinate& Location)
{
	//if (ContainsCoordinate(Location.X, Location.Y))
	//{
	//	if (m_Grid[Location.X][Location.Y]->GetObjectType() == GRID_OBJECT_TYPE::SPAWN)
	//	{
	//		AActor* spawn = m_Grid[Location.X][Location.Y]->RemoveObject();
	//		if (spawn)
	//		{
	//			spawn->Destroy();

	//			//Remove spawnLocation from list
	//			FGridCoordinate temp = m_SpawnLocations.back();
	//			for (auto& loc : m_SpawnLocations)
	//			{
	//				if (loc == Location)
	//				{
	//					loc = temp;
	//					m_SpawnLocations.pop_back();
	//					break;
	//				}
	//			}

	//			return true;
	//		}
	//	}
	//}

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
					NewPawn->ServerOnly_SetClassIndex(ClassIndex);
					NewPawn->ServerOnly_SetOwnerID(OwningPlayerID);
					m_PawnArray.Push(NewPawn);
					return true;
				}
			}
		}
	}
	return false;
}

bool AWorldGrid::ServerOnly_RemovePawn(const FGridCoordinate& Location, int pawnID)
{
	for (int index = 0; index < m_PawnArray.Num(); index++)
	{
		if (m_PawnArray[index]->GetPawnID() == pawnID)
		{
			m_PawnArray[index]->Destroy();
			m_PawnArray[index] = m_PawnArray[m_PawnArray.Num() - 1];
			m_PawnArray.Pop();

			return true;
		}
	}

	return false;
}

AMapPawn * AWorldGrid::ServerOnly_GetPawn(const FVector& Location, int pawnID)
{
	for (int index = 0; index < m_PawnArray.Num(); index++)
	{
		if (m_PawnArray[index]->GetPawnID() == pawnID)
		{

			return m_PawnArray[index];
		}
	}

	return nullptr;
}

bool AWorldGrid::ServerOnly_GetPath(const FGridCoordinate & Location, const FGridCoordinate & Destination, TArray<FGridCoordinate>& OutPath, int PawnID)
{
	//bool startExists = ContainsCoordinate(Location.X, Location.Y);
	//bool endExists = ContainsCoordinate(Destination.X, Destination.Y);

	//if (startExists && endExists)
	//{
	//	return UPathFinder::FindPath(m_Grid[Location.X][Location.Y], m_Grid[Destination.X][Destination.Y], OutPath, PawnID);
	//}

	return false;
}

FGridCoordinate AWorldGrid::ServerOnly_GetOpenSpawnLocation()const
{
	//for (const auto& loc : m_SpawnLocations)
	//{
	//	auto cell = m_Grid[loc.X][loc.Y];
	//	if (!cell->HasPawn())
	//	{
	//		return loc;
	//	}
	//}
	return FGridCoordinate();
}

void AWorldGrid::ServerOnly_EditCells(const TArray<FVector>& EditBoxVertices, const FCellEditInstruction& instructions)
{
	auto InstancedMesh = GetCellInstanceMesh(0);
	if (InstancedMesh)
	{
		TArray<int32> indexArray = InstancedMesh->GetInstancesOverlappingBox(FBox(EditBoxVertices), true);
		float deltaScaleZ = instructions.Height * CELL_STEP;

		UE_LOG(LogNotice, Warning, TEXT("<World Grid>: Begin Edit - %i elements in query"), indexArray.Num());
		for (const auto& index : indexArray)
		{
			FTransform cellTransform;
			if (InstancedMesh->GetInstanceTransform(index, cellTransform, true))
			{
				if (cellTransform.GetScale3D().Z + deltaScaleZ > 0)
				{
					cellTransform.SetScale3D(FVector(152.4f, 152.4f, cellTransform.GetScale3D().Z + deltaScaleZ));

					InstancedMesh->UpdateInstanceTransform(index, cellTransform, true);
				}
			}
		}

		InstancedMesh->MarkRenderStateDirty();
		UE_LOG(LogNotice, Warning, TEXT("<World Grid>: End Edit"));
	}
}


void AWorldGrid::ShowCollisions(bool value)
{
	bShowCollisions = value;
}

bool AWorldGrid::LoadMapObjects(const TArray<FSAVE_OBJECT>* GridSheet)
{
	if (GridSheet)
	{
		for (const auto& obj : *GridSheet)
		{
			switch (obj.Type)
			{
			case GRID_OBJECT_TYPE::NONE:
				break;
			case GRID_OBJECT_TYPE::INTERACTABLE:
				ServerOnly_AddBlockingObject(obj.ModelIndex, obj.Location);
				break;
			case GRID_OBJECT_TYPE::SPAWN:
				ServerOnly_AddSpawnLocation(obj.ModelIndex, obj.Location);
				break;
			case GRID_OBJECT_TYPE::PAWN:
				ServerOnly_AddPawn(obj.ModelIndex, obj.Location, 0);
				break;
			default:
				break;
			}
		}
		return true;
	}

	return false;
}

bool AWorldGrid::GeneratePlayArea(const FGridCoordinate& GridDimensions, const TArray<uint8>* HeightMap)
{
	if (!m_bMapIsLoaded)
	{
		// Generate the visual for each cell
		auto InstancedMesh = GetCellInstanceMesh(0);
		if (InstancedMesh)
		{
			m_UsedCellIndices.insert(0);
			for (int x = 0; x < GridDimensions.X; x++)
			{
				for (int y = 0; y < GridDimensions.Y; y++)
				{
					int height = FLOOR_HEIGHT_STEP;

					if (HeightMap)
					{
						const TArray<uint8>& arr = *HeightMap;
						int loc = (y * GridDimensions.X) + x;

						height = (int)(arr[loc]);
					}

					float CellSize = Conversions::Feet::ToCentimeters(CELL_WIDTH_FEET);
					FTransform cellTransform;
					cellTransform.SetTranslation(UGridFunctions::GridToWorldLocation(FGridCoordinate(x, y, 0)));
					cellTransform.SetScale3D(FVector(CellSize, CellSize, height * CELL_STEP));

					InstancedMesh->AddInstanceWorldSpace(cellTransform);
				}
			}

			m_GridDimensions = GridDimensions;
			m_bMapIsLoaded = true;

			return true;
		}
	}

	return false;
}

bool AWorldGrid::ContainsCoordinate(const FGridCoordinate & coordintate)
{
	return ContainsCoordinate(coordintate.X, coordintate.Y);
}

bool AWorldGrid::ContainsCoordinate(int x, int y)
{
	if (m_GridDimensions.X > x && x >= 0)
	{
		if (m_GridDimensions.Y> y && y >= 0)
		{
			return true;
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

	//if (bottomLeftExists)
	//{
	//	m_Grid[left][bottom]->Neigbor(AWorldGrid_Cell::NEIGHBOR::TOPRIGHT) = NewCell;
	//	NewCell->Neigbor(AWorldGrid_Cell::NEIGHBOR::BOTTOMLEFT) = m_Grid[left][bottom];
	//}
	//if (topLeftExists)
	//{
	//	m_Grid[left][top]->Neigbor(AWorldGrid_Cell::NEIGHBOR::BOTTOMRIGHT) = NewCell;
	//	NewCell->Neigbor(AWorldGrid_Cell::NEIGHBOR::TOPLEFT) = m_Grid[left][top];
	//}
	//if (leftExists)
	//{
	//	m_Grid[left][center.Y]->Neigbor(AWorldGrid_Cell::NEIGHBOR::RIGHT) = NewCell;
	//	NewCell->Neigbor(AWorldGrid_Cell::NEIGHBOR::LEFT) = m_Grid[left][center.Y];
	//} 
	//if (topExists)
	//{
	//	m_Grid[center.X][top]->Neigbor(AWorldGrid_Cell::NEIGHBOR::BOTTOM) = NewCell;
	//	NewCell->Neigbor(AWorldGrid_Cell::NEIGHBOR::TOP) = m_Grid[center.X][top];
	//}
}

void AWorldGrid::OnRep_BuildMap()
{
	// Only run if the server has identified the map
	if (!m_MapName.IsEmpty())
	{
		TArray<FSAVE_OBJECT> GridSheet;
		FString path = FString::Printf(TEXT("%sMaps/%s.map"), *FPaths::ProjectUserDir(), *m_MapName);
		UMapSaveFile* MapSaveFile = Cast<UMapSaveFile>(UBasicFunctions::LoadFile(path));
		if (MapSaveFile)
		{
			UE_LOG(LogNotice, Warning, TEXT("Map Loaded!"));
			UE_LOG(LogNotice, Warning, TEXT("Name: %s"), *MapSaveFile->MapName);
			UE_LOG(LogNotice, Warning, TEXT("Size: (%i, %i)"), MapSaveFile->MapSize.X, MapSaveFile->MapSize.Y);
			UE_LOG(LogNotice, Warning, TEXT("Number of Objects: %i"), MapSaveFile->ObjectList.Num());

			GeneratePlayArea(MapSaveFile->MapSize, &MapSaveFile->HeightMap);
		}
	}
}
