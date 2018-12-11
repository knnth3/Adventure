// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include "CoreMinimal.h"
#include "Basics.h"
#include "Saves/MapSaveFile.h"
#include "GameFramework/Actor.h"
#include "WorldGrid_Cell.generated.h"

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
	void SetParent(class AWorldGrid* parent);

	bool operator<(const AWorldGrid_Cell* b);

	int GetFCost()const;

	UFUNCTION(BlueprintCallable, Category = "Grid Cell")
	bool IsEmpty() const;

	UFUNCTION(BlueprintCallable, Category = "Grid Cell")
	bool IsOcupied() const;

	UFUNCTION(BlueprintCallable, Category = "Grid Cell")
	bool HasPawn() const;

	UFUNCTION(BlueprintCallable, Category = "Grid Cell")
	GRID_OBJECT_TYPE GetObjectType() const;

	UFUNCTION(BlueprintCallable, Category = "Grid Cell")
	bool AddObject(GRID_OBJECT_TYPE type, AActor* actor);

	UFUNCTION(BlueprintCallable, Category = "Grid Cell")
	bool AddPawn(AActor* actor);

	UFUNCTION(BlueprintCallable, Category = "Grid Cell")
	AActor* RemoveObject();

	UFUNCTION(BlueprintCallable, Category = "Grid Cell")
	AActor* RemovePawn(int pawnID = -1);

	UFUNCTION(BlueprintCallable, Category = "Grid Cell")
	void AddBlockingSpace();

	UFUNCTION(BlueprintCallable, Category = "Grid Cell")
	void RemoveBlockingSpace();

	UFUNCTION(BlueprintCallable, Category = "Grid Cell")
	AActor* GetObject();

	UFUNCTION(BlueprintCallable, Category = "Grid Cell")
	class AMapPawn* GetPawn(int PawnID) const;

	UFUNCTION(BlueprintCallable, Category = "Grid Cell")
	void GetPawns(TArray<AActor*>& pawns);

	UFUNCTION(BlueprintCallable, Category = "Grid Cell")
	void ClearCell(TArray<AActor*>& contents);

	AWorldGrid_Cell*& Neigbor(const NEIGHBOR& Location);
	std::list<AWorldGrid_Cell*> GetNeighbors();

	FGridCoordinate Location;
	AWorldGrid_Cell* Parent;
	int H_Cost;
	int G_Cost;

protected:

	UPROPERTY(BlueprintReadWrite, Category = "Visual Settings")
	class AWorldGrid* ParentGrid;


private:

	AActor* m_Object;
	int m_BlockingSpaceCount;
	GRID_OBJECT_TYPE m_ObjectType;
	std::vector<AActor*> m_Pawns;
	AWorldGrid_Cell* Neighbors[8];
	
};
