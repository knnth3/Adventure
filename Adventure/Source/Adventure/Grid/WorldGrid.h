// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <map>
#include <set>
#include "Basics.h"
#include "Saves/MapSaveFile.h"
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

	// Ping clients until clients connect successfully
	UFUNCTION(BlueprintCallable, Category = "World Grid")
	void ServerOnly_BeginMapPing();

	// Loads weapons and items to the inventory database
	UFUNCTION(BlueprintCallable, Category = "World Grid")
	void ServerOnly_AddToInventory(const TArray<FWeaponInfo>& Weapons, const TArray<FConsumableInfo>& Consumables);

	// Set map name (used to save map)
	UFUNCTION(BlueprintCallable, Category = "World Grid")
	void ServerOnly_SetMapName(const FString& MapName);

	// Set map name (used to save map)
	UFUNCTION(BlueprintCallable, Category = "World Grid")
	void ServerOnly_SetMapSize(const FGridCoordinate& MapSize);

	// Saves a map using the ActiveSaveFileName
	UFUNCTION(BlueprintCallable, Category = "World Grid")
	bool ServerOnly_SaveMap();

	// Resets the grid removing all object instances
	UFUNCTION(BlueprintCallable, Category = "World Grid")
	void ServerOnly_ResetGrid();

	// Adds a blocking object to the grid
	UFUNCTION(BlueprintCallable, Category = "World Grid")
	bool ServerOnly_AddBlockingObject(uint8 ClassIndex, const FTransform & transform);

	// Removes a blocking object from the grid
	UFUNCTION(BlueprintCallable, Category = "World Grid")
	bool ServerOnly_RemoveBlockingObjects(const TArray<FVector>& EditBoxVertices);

	// Adds a pawn to the grid
	UFUNCTION(BlueprintCallable, Category = "World Grid")
	bool ServerOnly_AddPawn(int ClassIndex, const FGridCoordinate & Location, int OwningPlayerID);

	// Removes a pawn from the grid
	UFUNCTION(BlueprintCallable, Category = "World Grid")
	bool ServerOnly_RemovePawn(int pawnID);

	// Gets a pawn using it's pawnID
	UFUNCTION(BlueprintCallable, Category = "World Grid")
	class AMapPawn* ServerOnly_GetPawn(const FVector& Location, int pawnID);

	// Edit a group of cells given a bounding box and instructions
	UFUNCTION(BlueprintCallable, Category = "World Grid")
	void ServerOnly_EditCells(const TArray<FVector>& EditBoxVertices, const FCellEditInstruction& instructions);

	// Builds a given location on the client given the location
	UFUNCTION(BlueprintCallable, Category = "World Grid")
	void BuildLocation(const FMapLocation& Data);

	// Generates an empty grid given a size
	UFUNCTION(BlueprintCallable, Category = "World Grid")
	void GenerateEmptyLocation(const FGridCoordinate& Size);

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	FGridCoordinate GetMapSize() const;

	UFUNCTION(BlueprintCallable, Category = "World Grid")
	FString GetMapName() const;

protected:

	// Called when actor is destroyed
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;

	UPROPERTY(EditAnywhere, Category = "Components")
	TArray<TSubclassOf<class AMapPawn>> MapPawnClasses;

	UPROPERTY(EditAnywhere, Category = "Components")
	TArray<TSubclassOf<class AInteractable>> InteractableClasses;

	// Callback function to signal when grid has been constructed
	UFUNCTION(BlueprintImplementableEvent, Category = "Cell Functions")
	void OnGridConstructed(const FGridCoordinate& NewGridDimensions);

	// Query function to get HISM from BP
	UFUNCTION(BlueprintImplementableEvent, Category = "Cell Functions")
	class UHierarchicalInstancedStaticMeshComponent* GetCellInstanceMesh(uint8 CellTypeIndex);

	// Query function to get HISM from BP
	UFUNCTION(BlueprintImplementableEvent, Category = "Cell Functions")
	class UHierarchicalInstancedStaticMeshComponent* GetObjectInstanceMesh(uint8 CellTypeIndex);

private:

	// Generates the grid given certain params
	bool GeneratePlayArea(const FGridCoordinate& GridDimensions, const TArray<uint8>* HeightMap = nullptr, 
		const TArray<uint8>* TextureMap = nullptr, const TArray<uint8>* Objects = nullptr, const TArray<FTransform>* ObjectTransforms = nullptr);

	// Function called to edit cell height
	void EditCellHeight(const TArray<FVector>& EditBoxVertices, float DeltaHeight);

	// Function called to edit object height
	void EditObjectHeight(const TArray<FVector>& EditBoxVertices, float DeltaHeight);

	// Function called to edit cell texture
	void EditCellTexture(const TArray<FVector>& EditBoxVertices, uint8 TextureIndex);

	UPROPERTY(Replicated)
	FGridCoordinate m_GridDimensions;

	UPROPERTY(Replicated)
	FString m_MapName;

	bool m_bMapFileExists;
	bool m_bMapIsLoaded;
	std::set<uint8> m_UsedCellIndices; 
	std::set<uint8> m_UsedObjectIndices;
	TArray<AMapPawn*> m_PawnInstances;
	class ANetConnectionVerifyNode* m_VerifyNode;

	// Client specific information
	FString m_CurrentLocation;
};
