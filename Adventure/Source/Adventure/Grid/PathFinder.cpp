// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "PathFinder.h"
#include "Algo/Reverse.h"

#define OCCUPIED_CELL_COST 400
#define TRACE_RADIUS 15
#define STEP_HEIGHT 76.2f

bool UPathFinder::FindPath(AActor* Pawn, FVector Destination, TArray<FVector>& OutPath)
{
	GridDataList GridData;
	FGridCoordinate StartLocation = Pawn->GetActorLocation();
	FGridCoordinate EndLocation = Destination;

	if (StartLocation != EndLocation)
	{
		GridEntityPtr start;
		GridEntityPtr end;
		FGridCoordinate Bounds = TracePlayArea(GridData, StartLocation, EndLocation, Pawn, start, end);

		if (start != nullptr && end != nullptr)
		{
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
		int extraCost = 0;

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

	for (int x = -1; x <= 1; x++) 
	{
		for (int y = -1; y <= 1; y++)
		{
			if (x + y != 0)
			{
				UE_LOG(LogNotice, Warning, TEXT("Attempting to move to (%i, %i)"), loc.X + x, loc.Y + y);
				GridEntityPtr Neighbor = GetElementAt(loc.X + x, loc.Y + y, GridData, Dimensions);
				AddIfAvailable(Current, Neighbor, Results);
			}
		}
	}

	UE_LOG(LogNotice, Warning, TEXT("%i Possible neighbors found."), Results.size());
	return Results;
}

FGridCoordinate UPathFinder::TracePlayArea(GridDataList & GridData, FGridCoordinate Start, FGridCoordinate End, AActor* actor, GridEntityPtr& StartPtr, GridEntityPtr& EndPtr)
{
	FVector actorLocation = actor->GetActorLocation();
	FGridCoordinate diff = End - Start;

	GridData.resize((TRACE_RADIUS * 2) + 1);
	for (int x = -TRACE_RADIUS; x <= TRACE_RADIUS; x++)
	{
		GridData[x + TRACE_RADIUS].resize((TRACE_RADIUS * 2) + 1);
		for(int y = -TRACE_RADIUS; y <= TRACE_RADIUS; y++)
		{
			FHitResult results;
			ETraceTypeQuery traceChannel = ETraceTypeQuery::TraceTypeQuery1;
			FVector s = UGridFunctions::GridToWorldLocation(FGridCoordinate(Start.X + x, Start.Y + y, actorLocation.Z + 152.0f));
			FVector f = UGridFunctions::GridToWorldLocation(FGridCoordinate(Start.X + x, Start.Y + y, actorLocation.Z - 152.0f));
			if (UBasicFunctions::TraceLine(s, f, actor->GetWorld(), &results, traceChannel, actor, true))
			{
				float fHeight = FMath::DivideAndRoundUp(results.ImpactPoint.Z, STEP_HEIGHT);
				GridData[x + TRACE_RADIUS][y + TRACE_RADIUS] = std::make_shared<GridEntry>();
				GridData[x + TRACE_RADIUS][y + TRACE_RADIUS]->Location = FGridCoordinate(x + TRACE_RADIUS, y + TRACE_RADIUS);
				GridData[x + TRACE_RADIUS][y + TRACE_RADIUS]->TrueLocation = results.ImpactPoint;
				GridData[x + TRACE_RADIUS][y + TRACE_RADIUS]->Parent = nullptr;
				GridData[x + TRACE_RADIUS][y + TRACE_RADIUS]->height = (uint8)fHeight;

				if (x == 0 && y == 0)
				{
					StartPtr = GridData[x + TRACE_RADIUS][y + TRACE_RADIUS];
				}

				if (x == diff.X && y == diff.Y)
				{
					EndPtr = GridData[x + TRACE_RADIUS][y + TRACE_RADIUS];
				}
			}
		}
	}

	return FGridCoordinate((TRACE_RADIUS * 2) + 1, (TRACE_RADIUS * 2) + 1);
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
	if (end)
	{
		int heightDif = abs((int)(start->height) - end->height);
		if (heightDif <= 1)
		{
			Array.push_back(end);
		}
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
