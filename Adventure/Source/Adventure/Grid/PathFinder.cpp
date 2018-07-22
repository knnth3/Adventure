// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "PathFinder.h"
#include "Algo/Reverse.h"

bool UPathFinder::FindPath(AWorldGrid * Grid, const FGridCoordinate & Start, const FGridCoordinate & End, TArray<FGridCoordinate>& OutPath)
{
	if (!Grid)
		return false;

	CellPtr start = Grid->At(Start);
	CellPtr end = Grid->At(End);

	if (!start || !end)
		return false;

	PRIORITY_QUEUE(Cell) openQueue;
	std::map<CoordinatePair, CellPtr> openSet;
	std::map<CoordinatePair, CellPtr> closedSet;

	openQueue.push(start);
	openSet[start->Location.toPair()] = start;


	while (!openQueue.empty())
	{
		auto current = openQueue.top();
		openQueue.pop();
		openSet.erase(current->Location.toPair());

		closedSet[current->Location.toPair()] = current;

		if (current == end)
		{
			//Finished
			OutPath = TraceParentOwnership(start, end);
			return true;
		}

		for (const auto& neighbor : current->GetEmptyNeighbors())
		{
			if (closedSet.find(neighbor->Location.toPair()) != closedSet.end())
			{
				continue;
			}

			int newG_Cost = current->G_Cost + GetDistance(current, neighbor);

			bool bInOpenSet = false;
			if (openSet.find(neighbor->Location.toPair()) != openSet.end())
			{
				bInOpenSet = true;
			}

			if (newG_Cost < neighbor->G_Cost || !bInOpenSet)
			{
				neighbor->G_Cost = newG_Cost;
				neighbor->H_Cost = GetDistance(neighbor, end);
				neighbor->Parent = current;

				if (!bInOpenSet)
				{
					openQueue.push(neighbor);
					openSet[neighbor->Location.toPair()] = neighbor;
				}
			}
		}
	}

	return false;
}

TArray<FGridCoordinate> UPathFinder::TraceParentOwnership(const CellPtr & begin, const CellPtr & end)
{
	TArray<FGridCoordinate> Path;
	CellPtr current = end;

	while (current != begin)
	{
		Path.Push(current->Location);
		current = current->Parent;
	}

	Algo::Reverse(Path);

	return Path;
}

int UPathFinder::GetDistance(const CellPtr& from, const CellPtr& to)
{
	if (from && to)
	{
		int xDistance = abs(from->Location.X - to->Location.X);
		int yDistance = abs(from->Location.Y - to->Location.Y);

		if (xDistance > yDistance)
			return (14 * yDistance) + 10 * (xDistance - yDistance);

		return (14 * xDistance) + 10 * (yDistance - xDistance);
	}

	return 0;
}