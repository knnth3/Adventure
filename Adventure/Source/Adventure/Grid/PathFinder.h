// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include <queue>
#include "CoreMinimal.h"
#include "Basics.h"
#include "WorldGrid.h"
#include "HAL/Runnable.h"

/**
 * 
 */

#define OCCUPIED_CELL_COST 400
#define TRACE_RADIUS 15
#define STEP_HEIGHT 76.2f


typedef std::shared_ptr<struct GridEntry> GridEntityPtr;
typedef std::vector<std::vector<GridEntityPtr>> GridDataList;

DECLARE_DELEGATE_TwoParams(FPathFoundDelegate, bool, TArray<FVector>);

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


class ADVENTURE_API FPathFinder : public FRunnable
{

public:

	FPathFinder(const FVector& StartLocation, const FVector& Destination, UWorld* World, FPathFoundDelegate& Callback);

	static FPathFinder* RequestFindPath(const FVector& StartLocation, const FVector& Destination, UWorld* World, FPathFoundDelegate& Callback);

	void CancelRequest();

private:

	bool m_bTargetFound;
	int id;
	UWorld* m_World;
	FRunnableThread* m_Thread;
	GridEntityPtr m_StartPtr;
	GridEntityPtr m_EndPtr;
	GridDataList m_GeneratedGridData;
	FGridCoordinate m_Bounds;
	FGridCoordinate m_StartLocation;
	FGridCoordinate m_EndLocation;
	TArray<FVector> m_TargetPath;
	FPathFoundDelegate m_Callback;
	FThreadSafeBool m_bForceClose;

	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();

	void TraceParentOwnership();
	int GetDistance(const GridEntityPtr begin, const GridEntityPtr end);
	std::vector<GridEntityPtr> GetTraversableNeighbors(GridEntityPtr Current);
	FGridCoordinate TracePlayArea();
	GridEntityPtr GetElementAt(int x, int y);
	bool AddIfAvailable(GridEntityPtr& Start, GridEntityPtr& End, std::vector<GridEntityPtr>& ValidNeighbors);
	void EndThreadedTask();

	void ThreadOnly_SubmitEndEvent();

};
