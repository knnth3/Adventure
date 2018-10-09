// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <map>
#include "WorldGrid_Cell.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldGrid.generated.h"

UCLASS()
class ADVENTURE_API AWorldGrid : public AActor
{
	GENERATED_BODY()
	
public:	

	AWorldGrid();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void ServerOnly_GenerateGrid(const FGridCoordinate& Dimensions, std::vector<std::vector<int>> GridSheet);

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	void ServerOnly_GenerateGrid(const FGridCoordinate& Dimensions);

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	void ServerOnly_ResetGrid();

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	bool ServerOnly_AddVisual(int ClassIndex, const FGridCoordinate & Location);

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	bool ServerOnly_RemoveVisual(const FGridCoordinate& Location);

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
	class AMapPawn* ServerOnly_GetPawn(int pawnID);

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	TArray<class AMapPawn*> ServerOnly_GetAllPawns(int pawnID);

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	bool ServerOnly_GetPath(const FGridCoordinate & Location, const FGridCoordinate & Destination, TArray<FGridCoordinate>& OutPath);

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	FGridCoordinate ServerOnly_GetOpenSpawnLocation()const;

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	FVector GetCenterLocation()const;

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

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Instanced Cells")
	class UInstancedStaticMeshComponent* GridCellsMesh;

	UPROPERTY(EditAnywhere, Category = "Out of bounds settings")
	class URuntimeMeshComponent* RuntimeMesh;

	UPROPERTY(EditAnywhere, Category = "Out of bounds settings")
	UMaterialInterface* GeneratedAreaMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Out of bounds settings")
	float GeneratedAreaWidth;

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

	UFUNCTION(BlueprintImplementableEvent, Category = "Out of bounds settings")
	float GetGeneratedHeightValue(const FVector2D& Location);

private:

	void GenerateEnvironment(const FGridCoordinate& GridDimensions);
	bool ContainsCoordinate(int x, int y);
	void ServerOnly_LinkCell(AWorldGrid_Cell* NewCell);
	void GenerateBackdrop(const FGridCoordinate& GridDimensions);
	float GetBaseWeight(float CurrentRadius, float MaxRadius);

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
	FVector m_CenterLocation;
	std::map<int, int> m_PlayerPawnCount;
	std::vector<FGridCoordinate> m_SpawnLocations;
	std::vector<class AInteractable*> m_Visuals;
	std::vector<class AMapPawn*> m_MapPawns;
	std::vector<class ASpawner*> m_Spawns;
	std::vector<std::vector<AWorldGrid_Cell*>> m_Grid;
};
