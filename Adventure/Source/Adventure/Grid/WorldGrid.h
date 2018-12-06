// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <map>
#include "WorldGrid_Cell.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldGrid.generated.h"

USTRUCT(BlueprintType)
struct FCellEditInstruction
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(BlueprintReadWrite, Category = "Properties")
	int Height;
};

UCLASS()
class ADVENTURE_API AWorldGrid : public AActor
{
	GENERATED_BODY()
	
public:	

	AWorldGrid();
	bool ServerOnly_GenerateGrid(const FString& MapName, const TArray<FSAVE_OBJECT>* GridSheet);

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	bool ServerOnly_SaveMap();

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	bool ServerOnly_LoadGrid(const FString& MapName);

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	bool ServerOnly_GenerateGrid(const FString& MapName);

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	void ServerOnly_ResetGrid();

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	bool ServerOnly_AddBlockingObject(int ClassIndex, const FGridCoordinate & Location);

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	bool ServerOnly_RemoveBlockingObject(const FGridCoordinate& Location);

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	bool ServerOnly_AddSpawnLocation(int ClassIndex, const FGridCoordinate & Location);

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	bool ServerOnly_RemoveSpawnLocation(const FGridCoordinate& Location);

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	bool ServerOnly_AddPawn(int ClassIndex, const FGridCoordinate & Location, int OwningPlayerID);

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	bool ServerOnly_RemovePawn(const FGridCoordinate& Location, int pawnID);

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	class AMapPawn* ServerOnly_GetPawn(const FVector& Location, int pawnID);

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	bool ServerOnly_GetPath(const FGridCoordinate & Location, const FGridCoordinate & Destination, TArray<FGridCoordinate>& OutPath, int PawnID);

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	FGridCoordinate ServerOnly_GetOpenSpawnLocation()const;

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	void ServerOnly_EditCells(const TArray<FVector>& EditBoxVertices, const FCellEditInstruction& instructions);

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	FVector GetCenterLocation()const;

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
	class UHierarchicalInstancedStaticMeshComponent* GetCellInstanceMesh(uint8 CellTypeIndex);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Settings")
	bool bShowCollisions;

private:

	bool LoadMapObjects(const TArray<FSAVE_OBJECT>* GridSheet);
	void GenerateEnvironment(const FGridCoordinate& GridDimensions);
	bool ContainsCoordinate(const FGridCoordinate& coordintate);
	bool ContainsCoordinate(int x, int y);
	void ServerOnly_LinkCell(AWorldGrid_Cell* NewCell);
	int8 GetIndexForCell(const FGridCoordinate& Location);

	UFUNCTION()
	void OnRep_HasBeenConstructed();

	UPROPERTY(ReplicatedUsing = OnRep_HasBeenConstructed)
	FGridCoordinate m_GridDimensions;

	template<typename T>
	int FindEmptyIndex(std::vector<T*>& list)
	{
		int newID = list.size();
		list.push_back(nullptr);
		return newID;
	}

	bool bHasBeenConstructed;
	FString m_MapName;
	FVector m_CenterLocation;
	std::map<int, int> m_PlayerPawnCount;
	std::vector<FGridCoordinate> m_SpawnLocations;
	std::vector<std::vector<AWorldGrid_Cell*>> m_Grid;
	std::map<CoordinatePair, AWorldGrid_Cell*> m_HighlightedCells;
	std::vector<std::vector<int8>> m_CellIndices;
};
