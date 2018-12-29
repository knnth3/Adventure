// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <map>
#include <set>
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
	bool ServerOnly_RemoveBlockingObjects(const TArray<FVector>& EditBoxVertices);

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	bool ServerOnly_AddPawn(int ClassIndex, const FGridCoordinate & Location, int OwningPlayerID);

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	bool ServerOnly_RemovePawn(int pawnID);

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	class AMapPawn* ServerOnly_GetPawn(const FVector& Location, int pawnID);

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	void ServerOnly_EditCells(const TArray<FVector>& EditBoxVertices, const FCellEditInstruction& instructions);

protected:

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;

	UPROPERTY(EditAnywhere, Category = "Components")
	TArray<TSubclassOf<class AMapPawn>> MapPawnClasses;

	UPROPERTY(EditAnywhere, Category = "Components")
	TArray<TSubclassOf<class AInteractable>> InteractableClasses;

	UFUNCTION(BlueprintImplementableEvent, Category = "Cell Functions")
	void OnGridConstructed(const FGridCoordinate& NewGridDimensions);

	UFUNCTION(BlueprintImplementableEvent, Category = "Cell Functions")
	class UHierarchicalInstancedStaticMeshComponent* GetCellInstanceMesh(uint8 CellTypeIndex);

	UFUNCTION(BlueprintImplementableEvent, Category = "Cell Functions")
	class UHierarchicalInstancedStaticMeshComponent* GetObjectInstanceMesh(uint8 CellTypeIndex);

private:

	bool GeneratePlayArea(const class UMapSaveFile* Save);

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

	bool m_bMapIsLoaded;
	FGridCoordinate m_GridDimensions;
	std::set<uint8> m_UsedCellIndices; 
	std::set<uint8> m_UsedObjectIndices;
	TArray<AMapPawn*> m_PawnInstances;
};
