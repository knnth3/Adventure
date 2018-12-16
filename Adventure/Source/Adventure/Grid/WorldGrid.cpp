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
	SaveGameInstance->TextureMap.SetNum(m_GridDimensions.X * m_GridDimensions.Y);

	// Save cell information
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
					float fHeight = FMath::DivideAndRoundUp(CellTransform.GetScale3D().Z, CELL_STEP);
					int height = (uint8)fHeight;

					SaveGameInstance->HeightMap[loc] = height;
					SaveGameInstance->TextureMap[loc] = index;
				}
			}
		}
	}

	// Save object information
	for (const auto& index : m_UsedObjectIndices)
	{
		auto InstancedMesh = GetObjectInstanceMesh(index);
		if (InstancedMesh)
		{
			int count = InstancedMesh->GetInstanceCount();
			for(int x = 0; x < count; x++)
			{
				SaveGameInstance->ObjectTransforms.Emplace();
				if (InstancedMesh->GetInstanceTransform(x, SaveGameInstance->ObjectTransforms.Last(),true))
				{
					SaveGameInstance->Objects.Push(index);
				}
				else
				{
					SaveGameInstance->ObjectTransforms.Pop();
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
	return GeneratePlayArea(MapSize);
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

bool AWorldGrid::ServerOnly_AddBlockingObject(uint8 ClassIndex, const FTransform & transform)
{
	UE_LOG(LogNotice, Warning, TEXT("Spawning Obeject for index: %i"), ClassIndex);
	auto InstancedMesh = GetObjectInstanceMesh(ClassIndex);
	if (InstancedMesh)
	{
		InstancedMesh->AddInstance(transform);
		m_UsedObjectIndices.insert(ClassIndex);
		return true;
	}
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

void AWorldGrid::ServerOnly_EditCells(const TArray<FVector>& EditBoxVertices, const FCellEditInstruction& instructions)
{
	if (instructions.Height == 0)
		EditCellTexture(EditBoxVertices, instructions.TextureIndex);
	else
		EditCellHeight(EditBoxVertices, instructions.Height * CELL_STEP);
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
				// ServerOnly_AddBlockingObject(obj.ModelIndex, obj.Location);
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

bool AWorldGrid::GeneratePlayArea(const UMapSaveFile * Save)
{
	return GeneratePlayArea(Save->MapSize, &Save->HeightMap, &Save->TextureMap, &Save->Objects, &Save->ObjectTransforms);
}

bool AWorldGrid::GeneratePlayArea(const FGridCoordinate& GridDimensions, const TArray<uint8>* HeightMap, 
	const TArray<uint8>* TextureMap, const TArray<uint8>* Objects, const TArray<FTransform>* ObjectTransforms)
{
	if (!m_bMapIsLoaded)
	{
		// Generate cells
		for (int x = 0; x < GridDimensions.X; x++)
		{
			for (int y = 0; y < GridDimensions.Y; y++)
			{
				int texture = 0;
				int height = FLOOR_HEIGHT_STEPS;

				if (HeightMap)
				{
					const TArray<uint8>& arr = *HeightMap;
					int loc = (y * GridDimensions.X) + x;

					height = (int)(arr[loc]);
				}

				if (TextureMap)
				{
					const TArray<uint8>& arr = *TextureMap;
					int loc = (y * GridDimensions.X) + x;

					texture = arr[loc];
				}

				// Generate the visual for each cell
				auto InstancedMesh = GetCellInstanceMesh(texture);
				if (InstancedMesh)
				{
					m_UsedCellIndices.insert(texture);

					float CellSize = Conversions::Feet::ToCentimeters(CELL_WIDTH_FEET);
					FTransform cellTransform;
					cellTransform.SetTranslation(UGridFunctions::GridToWorldLocation(FGridCoordinate(x, y, 0)));
					cellTransform.SetScale3D(FVector(CellSize, CellSize, height * CELL_STEP));

					InstancedMesh->AddInstanceWorldSpace(cellTransform);
				}

			}
		}

		// Generate Objects
		if (Objects && ObjectTransforms && Objects->Num() == ObjectTransforms->Num())
		{
			uint8 prevIndex = 255;
			UHierarchicalInstancedStaticMeshComponent* CurrentInstancedMesh = NULL;
			for (int x = 0; x < Objects->Num(); x++)
			{
				uint8 index = (*Objects)[x];
				if (prevIndex != index)
				{
					CurrentInstancedMesh = GetObjectInstanceMesh(index);
					m_UsedObjectIndices.insert(index);
				}

				CurrentInstancedMesh->AddInstanceWorldSpace((*ObjectTransforms)[x]);
			}
		}

		m_GridDimensions = GridDimensions;
		m_bMapIsLoaded = true;
		OnGridConstructed(GridDimensions);

		return true;

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

void AWorldGrid::EditCellHeight(const TArray<FVector>& EditBoxVertices, float DeltaHeight)
{
	// Edit cells
	for (const auto& meshIndex : m_UsedCellIndices)
	{
		auto InstancedMesh = GetCellInstanceMesh(meshIndex);
		if (InstancedMesh)
		{
			TArray<int32> indexArray = InstancedMesh->GetInstancesOverlappingBox(FBox(EditBoxVertices), true);

			for (const auto& index : indexArray)
			{
				FTransform CellTransform;
				if (InstancedMesh->GetInstanceTransform(index, CellTransform, true))
				{
					if (CellTransform.GetScale3D().Z + DeltaHeight > 0 && CellTransform.GetScale3D().Z + DeltaHeight <= CELL_STEP * MAX_HEIGHT_STEPS)
					{
						CellTransform.SetScale3D(FVector(152.4f, 152.4f, CellTransform.GetScale3D().Z + DeltaHeight));

						InstancedMesh->UpdateInstanceTransform(index, CellTransform, true);
					}
				}
			}

			InstancedMesh->MarkRenderStateDirty();

		}
	}

	// Edit objects as well
	EditObjectHeight(EditBoxVertices, DeltaHeight);
}

void AWorldGrid::EditObjectHeight(const TArray<FVector>& EditBoxVertices, float DeltaHeight)
{
	for (const auto& meshIndex : m_UsedObjectIndices)
	{
		auto InstancedMesh = GetObjectInstanceMesh(meshIndex);
		if (InstancedMesh)
		{
			TArray<int32> indexArray = InstancedMesh->GetInstancesOverlappingBox(FBox(EditBoxVertices), true);

			for (const auto& index : indexArray)
			{
				FTransform ObjectTransform;
				if (InstancedMesh->GetInstanceTransform(index, ObjectTransform, true))
				{
					if (ObjectTransform.GetTranslation().Z + DeltaHeight > 1)
						ObjectTransform.AddToTranslation(FVector(0, 0, DeltaHeight));
					InstancedMesh->UpdateInstanceTransform(index, ObjectTransform, true);
				}
			}

			InstancedMesh->MarkRenderStateDirty();

		}
	}
}

void AWorldGrid::EditCellTexture(const TArray<FVector>& EditBoxVertices, uint8 TextureIndex)
{
	TArray<FTransform> ChangedMeshes;
	UHierarchicalInstancedStaticMeshComponent* ChangeToMesh = nullptr;
	for (const auto& meshIndex : m_UsedCellIndices)
	{
		auto InstancedMesh = GetCellInstanceMesh(meshIndex);
		if (InstancedMesh)
		{
			TArray<int32> indexArray = InstancedMesh->GetInstancesOverlappingBox(FBox(EditBoxVertices), true);
			ChangeToMesh = GetCellInstanceMesh(TextureIndex);
			if (TextureIndex != meshIndex && ChangeToMesh)
			{
				// Grab all the transforms that will be replaced
				for (const auto& instanceIndex : indexArray)
				{
					FTransform cellTransform;
					if (InstancedMesh->GetInstanceTransform(instanceIndex, cellTransform, true))
					{
						ChangedMeshes.Push(cellTransform);
					}
				}

				// Erase after to avoid any wierd index changes
				InstancedMesh->RemoveInstances(indexArray);
			}
		}
	}

	if (ChangedMeshes.Num() != 0)
	{
		for (const auto& transform : ChangedMeshes)
		{
			ChangeToMesh->AddInstanceWorldSpace(transform);
		}

		ChangeToMesh->MarkRenderStateDirty();
		m_UsedCellIndices.insert(TextureIndex);
	}
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
			UE_LOG(LogNotice, Warning, TEXT("Number of Objects: %i"), MapSaveFile->Objects.Num());

			GeneratePlayArea(MapSaveFile);
		}
	}
}
