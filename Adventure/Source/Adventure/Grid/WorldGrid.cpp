// By: Eric Marquez. All information and code provided is free to use and can be used comercially.
// Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.
#include "WorldGrid.h"
#include "./Character/MapPawn.h"
#include "Interactable.h"
#include "Grid/GridEntity.h"
#include "Spawner.h"
#include "PathFinder.h"
#include "Components/InstancedStaticMeshComponent.h"

using namespace std;

AWorldGrid::AWorldGrid()
{
	bReplicates = true;
	bShowCollisions = false;
	bAlwaysRelevant = true;
	bReplicateMovement = false;
	bHasBeenConstructed = false;
	bGenerateBackDrop = true;
	GeneratedAreaWidth = 1000.0f;
	GeneratedAreaHeightRange = 1000.0f;
	GeneratedAreaPlayAreaRandomIntensity = 1.0f;
	GeneratedAreaTesselation = 20;

	GridCellsMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Focus"));
	GridCellsMesh->SetIsReplicated(true);
	RootComponent = GridCellsMesh;

	BackgroundTreesMesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Trees"));
	BackgroundTreesMesh->SetIsReplicated(true);
	BackgroundTreesMesh->SetupAttachment(RootComponent);
}

void AWorldGrid::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const 
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWorldGrid, m_GridDimensions);
}

bool AWorldGrid::ServerOnly_SaveMap()
{
	UMapSaveFile* SaveGameInstance = Cast<UMapSaveFile>(UGameplayStatics::CreateSaveGameObject(UMapSaveFile::StaticClass()));
	SaveGameInstance->MapName = m_MapName;
	SaveGameInstance->MapSize = m_GridDimensions;

	for (int x = 0; x < m_GridDimensions.X; x++)
	{
		for (int y = 0; y < m_GridDimensions.Y; y++)
		{
			FSAVE_OBJECT object;

			// If the cell has an interactable on it
			AActor* obj = m_Grid[x][y]->GetObject();
			IGridEntity* GEInterface = Cast<IGridEntity>(obj);
			if (GEInterface)
			{
				object.Location = { x,y };
				object.Type = GRID_OBJECT_TYPE::INTERACTABLE;
				object.ModelIndex = GEInterface->Execute_GetClassIndex(obj);
				SaveGameInstance->GridSheet.Push(object);
			}

			if (m_Grid[x][y]->HasPawn())
			{
				TArray<AActor*> pawns;
				m_Grid[x][y]->GetPawns(pawns);

				for (const auto& pawn : pawns)
				{
					GEInterface = Cast<IGridEntity>(pawn);
					AMapPawn* castedPawn = Cast<AMapPawn>(pawn);
					if (GEInterface)
					{
						object.Location = { x,y };
						object.Type = GRID_OBJECT_TYPE::PAWN;
						object.ModelIndex = GEInterface->Execute_GetClassIndex(pawn);
						object.OwnerName = "";
						SaveGameInstance->GridSheet.Push(object);
					}
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
	UMapSaveFile* MapSaveFile = Cast<UMapSaveFile>(UBasicFunctions::LoadFile(path));

	if (MapSaveFile)
	{
		UE_LOG(LogNotice, Warning, TEXT("Map Loaded!"));
		UE_LOG(LogNotice, Warning, TEXT("Name: %s"), *MapSaveFile->MapName);
		UE_LOG(LogNotice, Warning, TEXT("Size: (%i, %i)"), MapSaveFile->MapSize.X, MapSaveFile->MapSize.Y);
		UE_LOG(LogNotice, Warning, TEXT("Number of Objects: %i"), MapSaveFile->GridSheet.Num());

		m_GridDimensions.X = MapSaveFile->MapSize.X;
		m_GridDimensions.Y = MapSaveFile->MapSize.Y;

		return ServerOnly_GenerateGrid(MapName, MapSaveFile->MapSize, &MapSaveFile->GridSheet);
	}

	return false;
}

bool AWorldGrid::ServerOnly_GenerateGrid(const FString& MapName, const FGridCoordinate& GridDimensions, const TArray<FSAVE_OBJECT>* GridSheet)
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
					newCell->Initialize(FGridCoordinate(x, y));
					newCell->SetParent(this);
					m_Grid[x][y] = newCell;
					ServerOnly_LinkCell(newCell);
					cellCount++;
				}
			}
		}
	}

	if (cellCount == GridDimensions.X * GridDimensions.Y)
	{
		m_MapName = MapName;
		LoadMapObjects(GridSheet);
		FVector BottomRight = UGridFunctions::GridToWorldLocation(GridDimensions);
		m_CenterLocation = FVector(BottomRight.X * 0.5f, BottomRight.Y * 0.5f, 0.0f);
		m_GridDimensions = GridDimensions;
		OnRep_HasBeenConstructed();
		UE_LOG(LogNotice, Warning, TEXT("<Grid Generation>: Finished generating grid"));

		return true;
	}
	else
	{
		UE_LOG(LogNotice, Warning, TEXT("<Grid Generation>: Failed to generate grid"));

		return false;
	}
}

bool AWorldGrid::ServerOnly_GenerateGrid(const FString & MapName, const FGridCoordinate & Dimensions)
{
	return ServerOnly_GenerateGrid(MapName, Dimensions, nullptr);
}

void AWorldGrid::ServerOnly_ResetGrid()
{
	m_GridDimensions = FGridCoordinate(0, 0);

	for (auto& row : m_Grid)
	{
		for (auto& cell : row)
		{
			TArray<AActor*> contents;
			cell->ClearCell(contents);

			for (auto& obj : contents)
			{
				obj->Destroy();
			}

			cell->Destroy();
		}
	}

	m_Grid.clear();
	GridCellsMesh->ClearInstances();
}

bool AWorldGrid::ServerOnly_AddBlockingObject(int ClassIndex, const FGridCoordinate & Location)
{
	if (ContainsCoordinate(Location.X, Location.Y) && !m_Grid[Location.X][Location.Y]->IsOcupied())
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
					NewBlockingObject->ServerOnly_SetClassIndex(ClassIndex);

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
		if (m_Grid[Location.X][Location.Y]->GetObjectType() == GRID_OBJECT_TYPE::INTERACTABLE)
		{
			AActor* object = m_Grid[Location.X][Location.Y]->RemoveObject();
			if (object)
			{
				object->Destroy();
				return true;
			}
		}
	}

	return false;
}

bool AWorldGrid::ServerOnly_AddSpawnLocation(int ClassIndex, const FGridCoordinate & Location)
{
	if (ContainsCoordinate(Location.X, Location.Y) && !m_Grid[Location.X][Location.Y]->IsOcupied())
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
		if (m_Grid[Location.X][Location.Y]->GetObjectType() == GRID_OBJECT_TYPE::SPAWN)
		{
			AActor* spawn = m_Grid[Location.X][Location.Y]->RemoveObject();
			if (spawn)
			{
				spawn->Destroy();

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

				return true;
			}
		}
	}

	return false;
}

bool AWorldGrid::ServerOnly_AddPawn(int ClassIndex, const FGridCoordinate & Location, int OwningPlayerID)
{
	if (ContainsCoordinate(Location.X, Location.Y) && !m_Grid[Location.X][Location.Y]->IsOcupied())
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
		AActor* pawn = m_Grid[Location.X][Location.Y]->RemovePawn(pawnID);
		if (pawn)
		{
			pawn->Destroy();
			return true;
		}
	}

	return false;
}

AMapPawn * AWorldGrid::ServerOnly_GetPawn(const FVector& Location, int pawnID)
{
	FGridCoordinate GridLocation = Location;
	if (ContainsCoordinate(GridLocation.X, GridLocation.Y))
	{
		return m_Grid[GridLocation.X][GridLocation.Y]->GetPawn(pawnID);
	}

	return nullptr;
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
		if (!cell->HasPawn())
		{
			return loc;
		}
	}
	return FGridCoordinate();
}

FVector AWorldGrid::GetCenterLocation() const
{
	return m_CenterLocation;
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

bool AWorldGrid::ContainsCoordinate(const FGridCoordinate & coordintate)
{
	return ContainsCoordinate(coordintate.X, coordintate.Y);
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
	if (bGenerateBackDrop)
	{
		FProceduralMeshInfo Vertices;
		TArray<int32> Triangles;

		float width = (float)GeneratedAreaWidth;
		float height = (float)GeneratedAreaWidth;
		float gridWidth = abs(m_CenterLocation.X * 2.0f);
		float gridHeight = abs(m_CenterLocation.Y * 2.0f);
		float totalWidth = width + gridWidth;
		float totalHeight = height + gridHeight;

		FGridCoordinate gridDimensions = m_GridDimensions;
		FGridCoordinate landscapeDimensions(GeneratedAreaWidth, GeneratedAreaWidth);
		FGridCoordinate total = gridDimensions + (landscapeDimensions * 2);

		FVector LandscapeDims = UGridFunctions::GridToWorldLocation({ 0, 0 }) * FVector(-total.X, total.Y, 0.0f) * 2.0f;
		FVector PositionOffset = UGridFunctions::GridToWorldLocation({ 0, 0 }) * FVector(width + gridDimensions.X, -height, 0) * 2.0f;

		if (GeneratedAreaWidth)
		{
			MeshLibrary::GenerateGrid(Vertices, Triangles, GeneratedAreaTesselation, GeneratedAreaTesselation,
				LandscapeDims.X, LandscapeDims.Y, PositionOffset.X, PositionOffset.Y);

			for (auto& Position : Vertices.Positions)
			{
				float vertexDistance = FVector::Dist2D(m_CenterLocation, Position);
				float acceptableRadius = (gridWidth > gridHeight) ? gridWidth : gridHeight;

				float generatedHeight = GetGeneratedHeightValue(FVector2D(Position.X / GeneratedAreaPlayAreaRandomIntensity, Position.Y / GeneratedAreaPlayAreaRandomIntensity));
				float floorHeight = 0.0f;
				float finalHeight = generatedHeight;

				if (vertexDistance <= acceptableRadius)
				{
					finalHeight = floorHeight;
				}
				else
				{
					float weight = GetBaseWeight(vertexDistance - acceptableRadius, SmoothingRadius);
					finalHeight = (generatedHeight * weight);
				}

				Position.Z = finalHeight * GeneratedAreaHeightRange - 1.0f;
			}

			// Create the mesh section
			GenerateBackdropMeshSection(Vertices, Triangles, total);
		}
	}
}

float AWorldGrid::GetBaseWeight(float CurrentRadius, float MaxRadius)
{
	float ratio = (CurrentRadius / MaxRadius);
	float value = 1.0f / (1.0f + pow(EULERS_NUMBER, (-10.0f * ratio) + 5.0f));

	return value;
}

void AWorldGrid::OnRep_HasBeenConstructed()
{
	GenerateEnvironment(m_GridDimensions);
	bHasBeenConstructed = true;
}
