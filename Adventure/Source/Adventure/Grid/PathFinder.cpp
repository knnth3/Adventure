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
		FGridCoordinate Bounds = TracePlayArea(GridData, StartLocation, EndLocation, start, end);

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
					OutPath = TraceParentOwnership(start, end, StartLocation);

					return true;
				}

				for (const auto& neighbor : GetTraversableNeighbors(GridData, current, Bounds, StartLocation, Pawn->GetWorld()))
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

TArray<FVector> UPathFinder::TraceParentOwnership(GridEntityPtr begin, GridEntityPtr end, const FGridCoordinate& StartLocation)
{
	TArray<FVector> Path;
	GridEntityPtr current = end;

	while (current != begin)
	{
		FGridCoordinate offset(TRACE_RADIUS, TRACE_RADIUS);
		FGridCoordinate loc = (current->Location - offset) + StartLocation;
		FVector Location3D = UGridFunctions::GridToWorldLocation(loc);
		Location3D.Z = (float)current->Location.Z;

		Path.Push(Location3D);
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

std::vector<GridEntityPtr> UPathFinder::GetTraversableNeighbors(GridDataList GridData, GridEntityPtr Current, const FGridCoordinate& Dimensions, const FGridCoordinate& StartLocation, UWorld* World)
{
	std::vector<GridEntityPtr> Results;
	const FGridCoordinate& loc = Current->Location;

	// Get Direct Neighbors First
	GridEntityPtr Top = GetElementAt(loc.X + 0, loc.Y + 1, GridData, Dimensions);
	GridEntityPtr Bottom = GetElementAt(loc.X + 0, loc.Y - 1, GridData, Dimensions);
	GridEntityPtr Left = GetElementAt(loc.X - 1, loc.Y + 0, GridData, Dimensions);
	GridEntityPtr Right = GetElementAt(loc.X + 1, loc.Y + 0, GridData, Dimensions);

	bool TopAdded = AddIfAvailable(Current, Top, Results, StartLocation, World);
	bool BottomAdded = AddIfAvailable(Current, Bottom, Results, StartLocation, World);
	bool LeftAdded = AddIfAvailable(Current, Left, Results, StartLocation, World);
	bool RightAdded = AddIfAvailable(Current, Right, Results, StartLocation, World);

	if (TopAdded && RightAdded)
	{
		GridEntityPtr TopRight = GetElementAt(loc.X + 1, loc.Y + 1, GridData, Dimensions);
		AddIfAvailable(Current, TopRight, Results, StartLocation, World);
	}

	if (TopAdded && LeftAdded)
	{
		GridEntityPtr TopLeft = GetElementAt(loc.X - 1, loc.Y + 1, GridData, Dimensions);
		AddIfAvailable(Current, TopLeft, Results, StartLocation, World);
	}

	if (BottomAdded && RightAdded)
	{
		GridEntityPtr BottomRight = GetElementAt(loc.X + 1, loc.Y - 1, GridData, Dimensions);
		AddIfAvailable(Current, BottomRight, Results, StartLocation, World);
	}

	if (BottomAdded && LeftAdded)
	{
		GridEntityPtr BottomLeft = GetElementAt(loc.X - 1, loc.Y - 1, GridData, Dimensions);
		AddIfAvailable(Current, BottomLeft, Results, StartLocation, World);
	}

	return Results;
}

FGridCoordinate UPathFinder::TracePlayArea(GridDataList & GridData, FGridCoordinate Start, FGridCoordinate End, GridEntityPtr& StartPtr, GridEntityPtr& EndPtr)
{
	FGridCoordinate diff = End - Start;

	GridData.resize((TRACE_RADIUS * 2) + 1);
	for (int x = -TRACE_RADIUS; x <= TRACE_RADIUS; x++)
	{
		GridData[x + TRACE_RADIUS].resize((TRACE_RADIUS * 2) + 1);
		for(int y = -TRACE_RADIUS; y <= TRACE_RADIUS; y++)
		{
			GridData[x + TRACE_RADIUS][y + TRACE_RADIUS] = std::make_shared<GridEntry>();
			GridData[x + TRACE_RADIUS][y + TRACE_RADIUS]->Location = FGridCoordinate(x + TRACE_RADIUS, y + TRACE_RADIUS, 0);
			GridData[x + TRACE_RADIUS][y + TRACE_RADIUS]->Parent = nullptr;

			if (x == 0 && y == 0)
			{
				StartPtr = GridData[x + TRACE_RADIUS][y + TRACE_RADIUS];
				StartPtr->Location.Z = Start.Z;
			}
			else if (x == diff.X && y == diff.Y)
			{
				EndPtr = GridData[x + TRACE_RADIUS][y + TRACE_RADIUS];
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

bool UPathFinder::AddIfAvailable(GridEntityPtr & start, GridEntityPtr & end, std::vector<GridEntityPtr>& Array, const FGridCoordinate& StartLocation, UWorld* World)
{
	if (end)
	{
		if (end->Location.Z == 0)
		{
			FGridCoordinate offset(TRACE_RADIUS, TRACE_RADIUS);
			FGridCoordinate loc = (end->Location - offset) + StartLocation;
			float StartHeight = start->Location.Z;

			if (loc.X >= 0 && loc.Y >= 0)
			{
				FHitResult result;
				ETraceTypeQuery traceChannel = ETraceTypeQuery::TraceTypeQuery1;
				FVector s = UGridFunctions::GridToWorldLocation(FGridCoordinate(loc.X, loc.Y, StartHeight + (STEP_HEIGHT * 2.0f)));
				FVector f = UGridFunctions::GridToWorldLocation(FGridCoordinate(loc.X, loc.Y, StartHeight - (STEP_HEIGHT * 2.0f)));
				//FVector s = UGridFunctions::GridToWorldLocation(FGridCoordinate(loc.X, loc.Y, 10000));
				//FVector f = UGridFunctions::GridToWorldLocation(FGridCoordinate(loc.X, loc.Y, 0));
				if (UBasicFunctions::TraceLine(s, f, World, result, traceChannel, nullptr, true))
				{
					end->Location.Z = result.ImpactPoint.Z;
				}
				else
				{
					end->Location.Z = 0;
				}
			}
			else
			{
				end->Location.Z = -1;
			}

		}
		
		if (end->Location.Z != -1 && end->Location.Z != 0)
		{
			float heightDif = (float)abs((start->Location.Z) - end->Location.Z);
			if (heightDif <= STEP_HEIGHT)
			{
				Array.push_back(end);
				return true;
			}
		}

	}

	return false;
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
