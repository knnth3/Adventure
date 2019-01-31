// By: Eric Marquez. All information and code provided is free to use and can be used comercially.
// Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.
#include "WorldGrid.h"
#include "./Character/MapPawn.h"
#include "Interactable.h"
#include "PathFinder.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "DataTables/PawnDatabase.h"
#include "PlayerControllers/PC_Multiplayer.h"
#include "Character/NetConnectionVerifyNode.h"

using namespace std;

AWorldGrid::AWorldGrid()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
	bReplicateMovement = false;
	m_bMapIsLoaded = false;
	m_bMapFileExists = false;
	m_VerifyNode = nullptr;
	m_GridDimensions = FGridCoordinate(10, 10);
}

void AWorldGrid::ServerOnly_BeginMapPing()
{
	if (!m_VerifyNode)
	{
		m_VerifyNode = GetWorld()->SpawnActor<ANetConnectionVerifyNode>();
	}
}

void AWorldGrid::ServerOnly_AddToInventory(const TArray<FWeaponInfo>& Weapons, const TArray<FConsumableInfo>& Consumables)
{
	// Add all weapons
	for (const auto& weapon : Weapons)
	{
		UInventoryDatabase::AddWeaponToDatabase(weapon);
	}

	// Add all consumables
	for (const auto& consumable : Consumables)
	{
		UInventoryDatabase::AddConsumableToDatabase(consumable);
	}
}

//sets variables for replicaton over a network
void AWorldGrid::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWorldGrid, m_GridDimensions);
	DOREPLIFETIME(AWorldGrid, m_MapName);
}

void AWorldGrid::ServerOnly_SetMapName(const FString & MapName)
{
	m_MapName = MapName;
}

void AWorldGrid::ServerOnly_SetMapSize(const FGridCoordinate & MapSize)
{
	m_GridDimensions = MapSize;
}

bool AWorldGrid::ServerOnly_SaveMap()
{
	UMapSaveFile* SaveGameInstance = Cast<UMapSaveFile>(UGameplayStatics::CreateSaveGameObject(UMapSaveFile::StaticClass()));
	SaveGameInstance->MapName = m_MapName;
	SaveGameInstance->ActiveLocation = "Temp";
	SaveGameInstance->Consumables = UInventoryDatabase::GetAllConsumablesInDatabase();
	SaveGameInstance->Weapons = UInventoryDatabase::GetAllWeaponsInDatabase();

	FMapLocation newLocation;
	newLocation.Name = "Temp";
	newLocation.Size = m_GridDimensions;
	newLocation.HeightMap.SetNum(m_GridDimensions.X * m_GridDimensions.Y);
	newLocation.TextureMap.SetNum(m_GridDimensions.X * m_GridDimensions.Y);

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

					newLocation.HeightMap[loc] = height;
					newLocation.TextureMap[loc] = index;
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
				newLocation.ObjectTransforms.Emplace();
				if (InstancedMesh->GetInstanceTransform(x, newLocation.ObjectTransforms.Last(),true))
				{
					newLocation.Objects.Push(index);
				}
				else
				{
					newLocation.ObjectTransforms.Pop();
				}
			}
		}
	}

	FString path = FString::Printf(TEXT("%sMaps/%s.map"), *FPaths::ProjectUserDir(), *SaveGameInstance->MapName);

	UE_LOG(LogNotice, Warning, TEXT("Map saved at: %s"), *path);

	SaveGameInstance->Locations.Push(newLocation);
	UBasicFunctions::SaveGameEx(SaveGameInstance, path);
	return true;
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

	for (const auto& index : m_UsedObjectIndices)
	{
		auto InstancedMesh = GetObjectInstanceMesh(index);
		if (InstancedMesh)
		{
			InstancedMesh->ClearInstances();
		}
	}

	m_UsedCellIndices.clear();
	m_UsedObjectIndices.clear();
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

bool AWorldGrid::ServerOnly_RemoveBlockingObjects(const TArray<FVector>& EditBoxVertices)
{
	for (const auto& index : m_UsedObjectIndices)
	{
		auto InstancedMesh = GetObjectInstanceMesh(index);
		if (InstancedMesh)
		{
			TArray<int32> indexArray = InstancedMesh->GetInstancesOverlappingBox(FBox(EditBoxVertices), true);

			// Remove all found instances
			InstancedMesh->RemoveInstances(indexArray);
		}
	}

	return true;
}

bool AWorldGrid::ServerOnly_AddPawn(int ClassIndex, const FGridCoordinate & Location, int OwningPlayerID)
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
				NewPawn->ServerOnly_SetClassIndex(ClassIndex);
				NewPawn->ServerOnly_SetOwnerID(OwningPlayerID);
				m_PawnInstances.Push(NewPawn);
				return true;
			}
		}
	}
	return false;
}

bool AWorldGrid::ServerOnly_RemovePawn(int pawnID)
{
	for (int index = 0; index < m_PawnInstances.Num(); index++)
	{
		if (m_PawnInstances[index]->GetPawnID() == pawnID)
		{
			m_PawnInstances[index]->Destroy();
			m_PawnInstances[index] = m_PawnInstances[m_PawnInstances.Num() - 1];
			m_PawnInstances.Pop();

			return true;
		}
	}

	return false;
}

AMapPawn * AWorldGrid::ServerOnly_GetPawn(const FVector& Location, int pawnID)
{
	for (int index = 0; index < m_PawnInstances.Num(); index++)
	{
		if (m_PawnInstances[index]->GetPawnID() == pawnID)
		{

			return m_PawnInstances[index];
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

void AWorldGrid::BuildLocation(const struct FMapLocation& Data)
{
	GeneratePlayArea(
		Data.Size,
		&Data.HeightMap,
		&Data.TextureMap,
		&Data.Objects,
		&Data.ObjectTransforms);
}

void AWorldGrid::GenerateEmptyLocation(const FGridCoordinate& Size)
{
	GeneratePlayArea(Size);
}

FGridCoordinate AWorldGrid::GetMapSize() const
{
	return m_GridDimensions;
}

FString AWorldGrid::GetMapName() const
{
	return m_MapName;
}

void AWorldGrid::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UInventoryDatabase::ClearDatabase();
	UPawnDatabase::ClearDatabase();
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