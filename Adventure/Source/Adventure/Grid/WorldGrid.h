// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Basics.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldGrid.generated.h"

#define MAKE_CELL(x) CellPtr(new Cell(x))

typedef std::shared_ptr<class Cell> CellPtr;

class ADVENTURE_API Cell
{
public:
	//Enum to access neighbors
	enum CELL_NEIGHBOR
	{
		CELL_TOP,
		CELL_LEFT,
		CELL_RIGHT,
		CELL_BOTTOM,
		CELL_TOPLEFT,
		CELL_BOTTOMLEFT,
		CELL_TOPRIGHT,
		CELL_BOTTOMRIGHT
	};

public:
	Cell();
	Cell(const FGridCoordinate& Location);

	int F_Cost()const;
	bool operator<(const Cell& b);

	CellPtr& operator[](const CELL_NEIGHBOR& Location);
	CellPtr& GetNeighbor(const CELL_NEIGHBOR& Location);
	std::list<CellPtr> GetEmptyNeighbors();

	FGridCoordinate Location;
	CellPtr Parent;
	bool isOcupied;
	bool isBlockingSpace;
	int H_Cost;
	int G_Cost;

private:

	CellPtr Neighbors[8];
};

UCLASS()
class ADVENTURE_API AWorldGrid : public AActor
{
	GENERATED_BODY()
	
	friend class AMapPawn;
	
public:	
	// Sets default values for this actor's properties
	AWorldGrid();

	UFUNCTION(BlueprintCallable, Category = "WorldGrid")
	void Initialize(int Rows, int Columns, int HostID);

	UFUNCTION(BlueprintCallable, Category = "WorldGrid")
	FGridCoordinate GetDimensions()const;

	UFUNCTION(BlueprintCallable, Category = "WorldGrid")
	bool IsSpaceTraversable(const FGridCoordinate& Location)const;

	UFUNCTION(BlueprintCallable, Category = "WorldGrid")
	bool IsSpawnLocation(const FGridCoordinate& Location)const;

	UFUNCTION(BlueprintCallable, Category = "WorldGrid")
	bool IsFreeRoamActive()const;

	UFUNCTION(BlueprintCallable, Category = "WorldGrid")
	void BeginTurnBasedMechanics();

	UFUNCTION(BlueprintCallable, Category = "WorldGrid")
	void EndTurnBasedMechanics();

	UFUNCTION(BlueprintCallable, Category = "WorldGrid")
	bool MoveCharacter(const AConnectedPlayer* ConnectedPlayer, const FGridCoordinate& Destination, int PawnID = 0);

	UFUNCTION(BlueprintCallable, Category = "WorldGrid")
	bool RegisterPlayerController(class AConnectedPlayer* ConnectedPlayer, int& CharacterID);

	UFUNCTION(BlueprintCallable, Category = "WorldGrid")
	bool FindPath(const FGridCoordinate& Start, const FGridCoordinate& End, TArray<FGridCoordinate>& OutPath);

	CellPtr At(const int X, const int Y)const;
	CellPtr At(const FGridCoordinate& Location)const;


	bool AddCharacter(int PlayerID = -1, int ClassIndex = 0);
	bool AddBlockingSpace(const FGridCoordinate& Location);
	bool AddVisual(int ClassIndex, const FGridCoordinate& Location);
	bool AddSpawnLocation(int ClassIndex, const FGridCoordinate& Location);
	bool AddSpawnLocations(int ClassIndex, const TArray<FGridCoordinate>& Locations);

	bool RemoveCharacter(int PlayerID);
	bool RemoveVisual(const FGridCoordinate& Location);
	bool RemoveBlockingSpace(const FGridCoordinate& Location);
	bool RemoveSpawnLocation(const FGridCoordinate& Location);

	void ClearCharacters();
	void ClearBlockingSpaces();
	void ClearSpawnLocations();

protected:

	UPROPERTY(EditAnywhere, Category = "Spawnables")
	TArray<TSubclassOf<class ASpawner>> SpawnerClasses;

	UPROPERTY(EditAnywhere, Category = "Spawnables")
	TArray<TSubclassOf<class AMapPawn>> MapPawnClasses;

	UPROPERTY(EditAnywhere, Category = "Spawnables")
	TArray<TSubclassOf<class AInteractable>> InteractableClasses;

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* PlaneVisual;

	UPROPERTY(EditAnywhere)
	class UMaterialInterface* Material;

private:

	struct CharacterInfo
	{
		class AMapPawn* Pawn;
		class AConnectedPlayer* Player;
	};

	bool GetOpenSpawnLocation(FGridCoordinate& Location);
	void GenerateGrid();
	void SetupVisuals(const FGridCoordinate & GridDimensions);
	bool SetPosition(const FGridCoordinate& Location, const FGridCoordinate& Destination);

	UFUNCTION()
	void OnGridDimensionRep();

	UPROPERTY(replicatedUsing = OnGridDimensionRep)
	FGridCoordinate Dimensions;

	int OwnerID;
	int ActivePlayer;
	bool bInitialized;
	vector2D<CellPtr> LogicalGrid;
	std::map<int, CharacterInfo> Characters;
	std::map<CoordinatePair, class ASpawner*> SpawnLocations;
	std::map<CoordinatePair, class AActor*> VisualGridRefrences;

};
