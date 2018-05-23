// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <vector>
#include <list>

#include "Basics.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WorldGrid.generated.h"

typedef std::shared_ptr<class Cell> CellPtr;

USTRUCT()
struct FGridDimensions
{
	GENERATED_BODY()
	
	UPROPERTY()
	int Rows;

	UPROPERTY()
	int Columns;
};

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

	Cell(FGridCoordinate Location);

	int F_Cost()const;

	bool operator<(const Cell& b);

	CellPtr operator[](const CELL_NEIGHBOR& Location);

	CellPtr GetNeighbor(const CELL_NEIGHBOR& Location);

	std::list<CellPtr> GetEmptyNeighbors();

	FGridCoordinate Location;
	CellPtr Parent;
	bool Ocupied;
	int H_Cost;
	int G_Cost;

private:

	CellPtr Neighbors[8];
};

UCLASS()
class ADVENTURE_API AWorldGrid : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWorldGrid();

	CellPtr At(FGridCoordinate Location)const;

	//Sets up the grid with logic and visual objects
	UFUNCTION(BlueprintCallable, Category = "WorldGrid")
	void Initialize(int Rows, int Columns);

	//Order may vary depending on spawn location availability
	UFUNCTION(BlueprintCallable, Category = "WorldGrid")
	void SetSpawnLocations(const TArray<FGridCoordinate>& Locations);

	//False implies all spawn points are occupied
	UFUNCTION(BlueprintCallable, Category = "WorldGrid")
	bool GetOpenSpawnLocation(FGridCoordinate& GridLocation);

	UFUNCTION(BlueprintCallable, Category = "WorldGrid")
	bool IsOccupied(FGridCoordinate Location)const;

	UFUNCTION(BlueprintCallable, Category = "WorldGrid")
	bool MovePawn(FGridCoordinate Location, FGridCoordinate Destination);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* PlaneVisual;

	UPROPERTY(EditAnywhere)
	class UMaterialInterface* Material;

private:

	//Unreal Engine
	UFUNCTION()
	void OnScale_Rep();

	UFUNCTION()
	void OnDimensions_Rep();

	UPROPERTY(ReplicatedUsing = OnScale_Rep)
	FVector Scale;

	UPROPERTY(ReplicatedUsing = OnDimensions_Rep)
	FGridDimensions GridDimensions;

	void SetUpGridVisual();
	void SetUpGridLogical();

	//C++ standard
	bool bInitialized;
	vector2D<CellPtr> LogicalGrid;
	std::vector<FGridCoordinate> SpawnLocations;
};
