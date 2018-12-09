// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "PathFinder.h"
#include "Algo/Reverse.h"

#define OCCUPIED_CELL_COST 400

bool UPathFinder::FindPath(AActor* Pawn, FVector Destination, TArray<FVector>& OutPath)
{
	GridDataList GridData;
	FGridCoordinate StartLocation = Pawn->GetActorLocation();
	FGridCoordinate EndLocation = Destination;

	if (StartLocation != EndLocation)
	{
		FGridCoordinate Bounds = TracePlayArea(GridData, StartLocation, EndLocation, Pawn->GetWorld());
		auto start = GridData[0][0];
		auto end = GridData[Bounds.X - 1][Bounds.Y - 1];

		if (start != nullptr && end != nullptr)
		{
			start->height = StartLocation.Z / 50;
			PRIORITY_QUEUE(GridEntityPtr) openQueue;
			std::map<CoordinatePair, GridEntityPtr> openSet;
			std::map<CoordinatePair, GridEntityPtr> closedSet;

			openQueue.push(start);
			openSet[StartLocation.toPair()] = GridData[0][0];

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

				for (const auto& neighbor : GetTraversableNeighbors(GridData, current, Bounds))
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
		}
	}

	return false;
}

TArray<FVector> UPathFinder::TraceParentOwnership(GridEntityPtr begin, GridEntityPtr end)
{
	TArray<FVector> Path;
	GridEntityPtr current = end;

	while (current != begin)
	{
		Path.Push(current->TrueLocation);
		current = current->Parent;
	}

	Algo::Reverse(Path);

	return Path;
}

int UPathFinder::GetDistance(const GridEntityPtr begin, const GridEntityPtr end)
{
	if (begin && end)
	{
		int xDistance = abs(begin->Location.X - end->Location.X);
		int yDistance = abs(begin->Location.Y - end->Location.Y);

		uint8 HeightDifference = abs(end->height - begin->height);
		int extraCost = (HeightDifference > 1) ? OCCUPIED_CELL_COST : 0;

		if (xDistance > yDistance)
			return (14 * yDistance) + 10 * (xDistance - yDistance) + extraCost;

		return (14 * xDistance) + 10 * (yDistance - xDistance) + extraCost;
	}

	return 0;
}

std::vector<GridEntityPtr> UPathFinder::GetTraversableNeighbors(GridDataList GridData, GridEntityPtr Current, const FGridCoordinate& Dimensions)
{
	std::vector<GridEntityPtr> Results;
	const FGridCoordinate& loc = Current->Location;
	GridEntityPtr Top          = GetElementAt(loc.X, loc.Y - 1, GridData, Dimensions);
	GridEntityPtr Bottom       = GetElementAt(loc.X, loc.Y + 1, GridData, Dimensions);
	GridEntityPtr Left         = GetElementAt(loc.X - 1, loc.Y, GridData, Dimensions);
	GridEntityPtr Right		= GetElementAt(loc.X + 1, loc.Y, GridData, Dimensions);
	GridEntityPtr TopRight		= GetElementAt(loc.X + 1, loc.Y - 1, GridData, Dimensions);
	GridEntityPtr TopLeft		= GetElementAt(loc.X - 1, loc.Y - 1, GridData, Dimensions);
	GridEntityPtr BottomRight	= GetElementAt(loc.X + 1, loc.Y + 1, GridData, Dimensions);
	GridEntityPtr BottomLeft   = GetElementAt(loc.X - 1, loc.Y + 1, GridData, Dimensions);

	Top         ? AddIfAvailable(Current, Top        , Results) : 0;
	Bottom      ? AddIfAvailable(Current, Bottom     , Results) : 0;
	Left        ? AddIfAvailable(Current, Left       , Results) : 0;
	Right       ? AddIfAvailable(Current, Right      , Results) : 0;
	TopRight    ? AddIfAvailable(Current, TopRight   , Results) : 0;
	TopLeft     ? AddIfAvailable(Current, TopLeft    , Results) : 0;
	BottomRight ? AddIfAvailable(Current, BottomRight, Results) : 0;
	BottomLeft  ? AddIfAvailable(Current, BottomLeft , Results) : 0;

	return Results;
}

FGridCoordinate UPathFinder::TracePlayArea(GridDataList & GridData, FGridCoordinate Start, FGridCoordinate End, UWorld* World)
{
	FGridCoordinate LinearPath = End - Start;
	int XMax = abs(LinearPath.X) + 1;
	int YMax = abs(LinearPath.Y) + 1;
	int xDir = LinearPath.X ? (LinearPath.X / abs(LinearPath.X)) : 0;
	int yDir = LinearPath.Y ? (LinearPath.Y / abs(LinearPath.Y)) : 0;

	GridData.resize(XMax); 
	for (int x = 0; x < XMax; x++)
	{
		GridData[x].resize(YMax);
		for(int y = 0; y < YMax; y++)
		{
			FHitResult results;
			ETraceTypeQuery traceChannel = ETraceTypeQuery::TraceTypeQuery1;
			FVector s = UGridFunctions::GridToWorldLocation(FGridCoordinate(Start.X + (x*xDir), Start.Y + (y*yDir), 100000000));
			FVector f = UGridFunctions::GridToWorldLocation(FGridCoordinate(Start.X + (x*xDir), Start.Y + (y*yDir), 0));
			if (UBasicFunctions::TraceLine(s, f, World, &results, true, traceChannel))
			{
				GridData[x][y] = std::make_shared<GridEntry>();
				GridData[x][y]->Location = FGridCoordinate(x, y);
				GridData[x][y]->TrueLocation = results.ImpactPoint;
				GridData[x][y]->Parent = nullptr;
				GridData[x][y]->height = (results.ImpactPoint.Z / 50);
			}
		}
	}

	return FGridCoordinate(XMax, YMax);
}

GridEntityPtr UPathFinder::GetElementAt(int x, int y, GridDataList GridData, const FGridCoordinate& Dimensions)
{
	if (Dimensions.X > x && x >= 0)
	{
		if (Dimensions.Y > y && y >= 0)
		{
			return GridData[x][y];
		}
	}

	return nullptr;
}

void UPathFinder::AddIfAvailable(GridEntityPtr & start, GridEntityPtr & end, std::vector<GridEntityPtr>& Array)
{
	int heightDif = abs((int)(start->height) - end->height);
	if (heightDif <= 1)
	{
		Array.push_back(end);
	}
}

void GridEntry::SetParent(GridEntityPtr parent)
{
	Parent = parent;
}

bool GridEntry::operator<(const GridEntry b)
{
	if (GetFCost() == b.GetFCost())
	{
		return H_Cost <= b.H_Cost;
	}

	return GetFCost() < b.GetFCost();
	return false;
}

int GridEntry::GetFCost() const
{
	return H_Cost + G_Cost;
}
