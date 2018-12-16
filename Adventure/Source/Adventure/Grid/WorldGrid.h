// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <map>
#include <set>
#include "WorldGrid_Cell.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldGrid.generated.h"

#define CELL_STEP 152.4f
#define FLOOR_HEIGHT_STEPS 3
#define MAX_HEIGHT_STEPS (10 + FLOOR_HEIGHT_STEPS)

USTRUCT(BlueprintType)
struct FCellEditInstruction
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(BlueprintReadWrite, Category = "Properties")
	int Height;

	UPROPERTY(BlueprintReadWrite, Category = "Properties")
	uint8 TextureIndex;
};

UCLASS()
class ADVENTURE_API AWorldGrid : public AActor
{
	GENERATED_BODY()
	
public:	

	AWorldGrid();

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	bool ServerOnly_GenerateGrid(const FString& MapName, const FGridCoordinate& MapSize);

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	bool ServerOnly_SaveMap();

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	bool ServerOnly_LoadGrid(const FString& MapName);

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	void ServerOnly_ResetGrid();

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	bool ServerOnly_AddBlockingObject(uint8 ClassIndex, const FTransform & transform);

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	bool ServerOnly_RemoveBlockingObject(const FGridCoordinate& Location);

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	bool ServerOnly_AddPawn(int ClassIndex, const FGridCoordinate & Location, int OwningPlayerID);

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	bool ServerOnly_RemovePawn(const FGridCoordinate& Location, int pawnID);

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	class AMapPawn* ServerOnly_GetPawn(const FVector& Location, int pawnID);

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	void ServerOnly_EditCells(const TArray<FVector>& EditBoxVertices, const FCellEditInstruction& instructions);

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	void ShowCollisions(bool value);

protected:

	// Spawner Components
	UPROPERTY(EditAnywhere, Category = "Components")
	TArray<TSubclassOf<class ASpawner>> SpawnerClasses;

	UPROPERTY(EditAnywhere, Category = "Components")
	TArray<TSubclassOf<class AMapPawn>> MapPawnClasses;

	UPROPERTY(EditAnywhere, Category = "Components")
	TArray<TSubclassOf<class AInteractable>> InteractableClasses;

	UPROPERTY(EditAnywhere, Category = "Components")
	TArray<TSubclassOf<class AWorldGrid_Cell>> CellClasses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Out of bounds settings")
	bool bGenerateBackDrop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Out of bounds settings")
	int GeneratedAreaWidth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Out of bounds settings")
	float GeneratedAreaTesselation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Out of bounds settings")
	float GeneratedAreaHeightRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Out of bounds settings")
	float GeneratedAreaPlayAreaRandomIntensity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Out of bounds settings")
	float SmoothingRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Out of bounds settings")
	bool bConfineCameraToPlayArea;

	UFUNCTION(BlueprintImplementableEvent, Category = "Cell Functions")
	void OnGridConstructed(const FGridCoordinate& NewGridDimensions);

	UFUNCTION(BlueprintImplementableEvent, Category = "Cell Functions")
	class UHierarchicalInstancedStaticMeshComponent* GetCellInstanceMesh(uint8 CellTypeIndex);

	UFUNCTION(BlueprintImplementableEvent, Category = "Cell Functions")
	class UHierarchicalInstancedStaticMeshComponent* GetObjectInstanceMesh(uint8 CellTypeIndex);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Settings")
	bool bShowCollisions;

private:

	bool LoadMapObjects(const TArray<FSAVE_OBJECT>* GridSheet);

	bool GeneratePlayArea(const UMapSaveFile* Save);

	bool GeneratePlayArea(const FGridCoordinate& GridDimensions, const TArray<uint8>* HeightMap = nullptr, 
		const TArray<uint8>* TextureMap = nullptr, const TArray<uint8>* Objects = nullptr, const TArray<FTransform>* ObjectTransforms = nullptr);

	bool ContainsCoordinate(const FGridCoordinate& coordintate);

	bool ContainsCoordinate(int x, int y);

	void EditCellHeight(const TArray<FVector>& EditBoxVertices, float DeltaHeight);

	void EditObjectHeight(const TArray<FVector>& EditBoxVertices, float DeltaHeight);

	void EditCellTexture(const TArray<FVector>& EditBoxVertices, uint8 TextureIndex);

	UFUNCTION()
	void OnRep_BuildMap();

	UPROPERTY(ReplicatedUsing = OnRep_BuildMap)
	FString m_MapName;

	template<typename T>
	int FindEmptyIndex(std::vector<T*>& list)
	{
		int newID = list.size();
		list.push_back(nullptr);
		return newID;
	}

	bool m_bMapIsLoaded;
	FGridCoordinate m_GridDimensions;
	std::set<uint8> m_UsedCellIndices; 
	std::set<uint8> m_UsedObjectIndices;
	std::map<int, int> m_PlayerPawnCount;
	TArray<AMapPawn*> m_PawnArray;
};
