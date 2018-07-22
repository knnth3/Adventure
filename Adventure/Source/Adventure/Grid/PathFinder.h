// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#define PRIORITY_QUEUE(T) std::priority_queue<std::shared_ptr<T>, std::vector<std::shared_ptr<T>>, shared_ptr_compare<T>>

#include <queue>
#include "CoreMinimal.h"
#include "Basics.h"
#include "WorldGrid.h"
#include "PathFinder.generated.h"

/**
 * 
 */

UCLASS()
class ADVENTURE_API UPathFinder : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, Category = "Path Finder")
	static bool FindPath(class AWorldGrid* Grid, const FGridCoordinate& Start, const FGridCoordinate& End, TArray<FGridCoordinate>& OutPath);

private:

	static TArray<FGridCoordinate> TraceParentOwnership(const CellPtr& begin, const CellPtr& end);
	static int GetDistance(const CellPtr & from, const CellPtr & to);

};
