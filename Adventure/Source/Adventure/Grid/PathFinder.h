// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include <queue>
#include "CoreMinimal.h"
#include "Basics.h"
#include "WorldGrid.h"
#include "PathFinder.generated.h"

/**
 * 
 */

typedef std::shared_ptr<struct GridEntry> GridEntityPtr;
typedef std::vector<std::vector<GridEntityPtr>> GridDataList;

struct GridEntry
{
	void SetParent(GridEntityPtr parent);

	bool operator<(const GridEntry b);

	int GetFCost()const;

	FGridCoordinate Location;
	GridEntityPtr Parent = nullptr;
	int H_Cost = 0;
	int G_Cost = 0;
};

struct CompareEntities
{
	bool operator () (std::shared_ptr<GridEntry> a, std::shared_ptr<GridEntry> b) const
	{
		return !(*a < *b);
	}
};

#define PRIORITY_QUEUE(T) std::priority_queue<T, std::vector<T>, CompareEntities>

UCLASS()
class ADVENTURE_API UPathFinder : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, Category = "Path Finder")
	static bool FindPath(AActor* Pawn, FVector Destination, TArray<FVector>& OutPath);

private:

	static TArray<FVector> TraceParentOwnership(GridEntityPtr begin, GridEntityPtr end, const FGridCoordinate& StartLocation);
	static int GetDistance(const GridEntityPtr begin, const GridEntityPtr end);
	static std::vector<GridEntityPtr> GetTraversableNeighbors(GridDataList GridData, GridEntityPtr Current, const FGridCoordinate& Dimensions, const FGridCoordinate& StartLocation, UWorld* World);
	static FGridCoordinate TracePlayArea(GridDataList & GridData, FGridCoordinate Start, FGridCoordinate End, GridEntityPtr& StartPtr, GridEntityPtr& EndPtr);
	static GridEntityPtr GetElementAt(int x, int y, GridDataList GridData, const FGridCoordinate& Dimensions);
	static bool AddIfAvailable(GridEntityPtr& start, GridEntityPtr& end, std::vector<GridEntityPtr>& Array, const FGridCoordinate& StartLocation, UWorld* World);

};
