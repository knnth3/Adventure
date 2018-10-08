// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#define PRIORITY_QUEUE(T) std::priority_queue<T*, std::vector<T*>, ptr_compare<T>>

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
	static bool FindPath(AWorldGrid_Cell* begin, AWorldGrid_Cell* end, TArray<FGridCoordinate>& OutPath);

private:

	static TArray<FGridCoordinate> TraceParentOwnership(AWorldGrid_Cell* begin, AWorldGrid_Cell* end);
	static int GetDistance(const AWorldGrid_Cell* begin, const AWorldGrid_Cell* end);

};
