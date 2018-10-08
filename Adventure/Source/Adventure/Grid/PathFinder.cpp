// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "PathFinder.h"
#include "Algo/Reverse.h"

#define OCCUPIED_CELL_COST 400

bool UPathFinder::FindPath(AWorldGrid_Cell* start, AWorldGrid_Cell* end, TArray<FGridCoordinate>& OutPath)
{
	if (!start || !end)
		return false;

	PRIORITY_QUEUE(AWorldGrid_Cell) openQueue;
	std::map<CoordinatePair, AWorldGrid_Cell*> openSet;
	std::map<CoordinatePair, AWorldGrid_Cell*> closedSet;

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

		for (const auto& neighbor : current->GetTraversableNeighbors())
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

TArray<FGridCoordinate> UPathFinder::TraceParentOwnership(AWorldGrid_Cell* begin, AWorldGrid_Cell* end)
{
	TArray<FGridCoordinate> Path;
	AWorldGrid_Cell* current = end;

	while (current != begin)
	{
		Path.Push(current->Location);
		current = current->Parent;
	}

	Algo::Reverse(Path);

	return Path;
}

int UPathFinder::GetDistance(const AWorldGrid_Cell* begin, const AWorldGrid_Cell* end)
{
	if (begin && end)
	{
		int xDistance = abs(begin->Location.X - end->Location.X);
		int yDistance = abs(begin->Location.Y - end->Location.Y);
		int extraCost = (end->IsOcupied()) ? OCCUPIED_CELL_COST : 0;

		if (xDistance > yDistance)
			return (14 * yDistance) + 10 * (xDistance - yDistance) + extraCost;

		return (14 * xDistance) + 10 * (yDistance - xDistance) + extraCost;
	}

	return 0;
}