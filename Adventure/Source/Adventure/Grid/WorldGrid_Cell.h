// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include "CoreMinimal.h"
#include "Basics.h"
#include "GameFramework/Actor.h"
#include "WorldGrid_Cell.generated.h"

USTRUCT(BlueprintType)
struct FCellObject
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, Category = "GridCoordinate")
	int ObjectID;

	UPROPERTY(BlueprintReadWrite, Category = "GridCoordinate")
	int ClassIndex;
};

UCLASS()
class ADVENTURE_API AWorldGrid_Cell : public AActor
{
	GENERATED_BODY()
	
public:
	//Enum to access neighbors
	enum NEIGHBOR
	{
		TOP,
		LEFT,
		RIGHT,
		BOTTOM,
		TOPLEFT,
		BOTTOMLEFT,
		TOPRIGHT,
		BOTTOMRIGHT
	};

public:
	AWorldGrid_Cell();
	void Initialize(const FGridCoordinate& newLocation);
	virtual void BeginPlay() override;

	bool operator<(const AWorldGrid_Cell* b);

	int GetFCost()const;

	UFUNCTION(BlueprintCallable, Category = "Grid Cell")
	bool IsOcupied() const;

	UFUNCTION(BlueprintCallable, Category = "Grid Cell")
	bool SetTraversable(bool value);

	UFUNCTION(BlueprintCallable, Category = "Grid Cell")
	bool IsTraversable() const;

	UFUNCTION(BlueprintCallable, Category = "Grid Cell")
	bool AddVisual(FCellObject object);

	UFUNCTION(BlueprintCallable, Category = "Grid Cell")
	int RemoveVisual();

	UFUNCTION(BlueprintCallable, Category = "Grid Cell")
	bool AddBlockingObject(FCellObject object);

	UFUNCTION(BlueprintCallable, Category = "Grid Cell")
	int RemoveBlockingObject();

	UFUNCTION(BlueprintCallable, Category = "Grid Cell")
	bool AddPawn(FCellObject object);

	UFUNCTION(BlueprintCallable, Category = "Grid Cell")
	int RemovePawn(int pawnID);

	UFUNCTION(BlueprintCallable, Category = "Grid Cell")
	bool AddSpawner(FCellObject object);

	UFUNCTION(BlueprintCallable, Category = "Grid Cell")
	int RemoveSpawner();

	UFUNCTION(BlueprintCallable, Category = "Grid Cell")
	void AddBlockingSpace();

	UFUNCTION(BlueprintCallable, Category = "Grid Cell")
	void RemoveBlockingSpace();

	UFUNCTION(BlueprintCallable, Category = "Grid Cell")
	bool ContainsPawn()const;

	UFUNCTION(BlueprintCallable, Category = "Grid Cell")
	bool ContainsVisuals()const;

	UFUNCTION(BlueprintCallable, Category = "Grid Cell")
	bool ContainsBlockingObject()const;

	UFUNCTION(BlueprintCallable, Category = "Grid Cell")
	bool ContainsSpawner()const;

	AWorldGrid_Cell*& Neigbor(const NEIGHBOR& Location);
	std::list<AWorldGrid_Cell*> GetTraversableNeighbors();

	FGridCoordinate Location;
	AWorldGrid_Cell* Parent;
	int H_Cost;
	int G_Cost;

private:

	int RemoveElement(int index, std::vector<FCellObject>& list);
	int GetPawnIndex(int pawnID, std::vector<FCellObject>& list);

	bool bTraversable;
	bool bBlockingObject;
	int m_BlockingSpaceCount;
	FCellObject m_Visual;
	FCellObject m_BlockingObject;
	FCellObject m_Spawner;
	std::vector<FCellObject> m_Pawns;
	AWorldGrid_Cell* Neighbors[8];

	
	
};
