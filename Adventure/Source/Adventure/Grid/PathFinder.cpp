// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "PathFinder.h"
#include "Algo/Reverse.h"
#include "Async.h"

FPathFinder::FPathFinder(const FVector & StartLocation, const FVector & Destination, UWorld * World, FPathFoundDelegate & Callback)
{
	static int ClassID = 0;

	ClassID = (ClassID + 1) % 1000;
	id = ClassID;
	m_bTargetFound = false;
	m_bForceClose = false;
	m_World = World;
	m_StartLocation = StartLocation; 
	m_EndLocation = Destination;
	m_Callback = Callback;
	m_StartPtr = nullptr;
	m_EndPtr = nullptr;

	// Start thread
	FString NewThreadName = "FPathFinder" + FString::FromInt(ClassID);

	// UE_LOG(LogNotice, Warning, TEXT("<%s>: Starting trace..."), *NewThreadName);
	m_Thread = FRunnableThread::Create(this, *NewThreadName, 0, TPri_BelowNormal);
}

FPathFinder* FPathFinder::RequestFindPath(const FVector& StartLocation, const FVector& Destination, UWorld* World, FPathFoundDelegate& Callback)
{
	if (World)
	{
		FPathFinder* NewPathFindRequest = new FPathFinder(StartLocation, Destination, World, Callback);
		return NewPathFindRequest;
	}

	return nullptr;
}

void FPathFinder::CancelRequest()
{
	Stop();
}

bool FPathFinder::Init()
{
	if (m_StartLocation != m_EndLocation)
	{
		m_Bounds = TracePlayArea();

		if (m_StartPtr != nullptr && m_EndPtr != nullptr)
			return true;
	}

	return false;
}

uint32 FPathFinder::Run()
{
	PRIORITY_QUEUE(GridEntityPtr) openQueue;
	std::map<CoordinatePair, GridEntityPtr> openSet;
	std::map<CoordinatePair, GridEntityPtr> closedSet;

	openQueue.push(m_StartPtr);
	openSet[m_StartLocation.toPair()] = m_GeneratedGridData[0][0];

	while (!openQueue.empty() && !m_bForceClose)
	{
		auto current = openQueue.top();
		openQueue.pop();
		openSet.erase(current->Location.toPair());

		closedSet[current->Location.toPair()] = current;

		if (current == m_EndPtr)
		{
			//Finished
			m_bTargetFound = true;

			TraceParentOwnership();
			ThreadOnly_SubmitEndEvent();
			return 0;
		}

		for (const auto& neighbor : GetTraversableNeighbors(current))
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
				neighbor->H_Cost = GetDistance(neighbor, m_EndPtr);
				neighbor->Parent = current;

				if (!bInOpenSet)
				{
					openQueue.push(neighbor);
					openSet[neighbor->Location.toPair()] = neighbor;
				}
			}
		}
	}

	m_bTargetFound = false;
	ThreadOnly_SubmitEndEvent();
	return 0;
}

void FPathFinder::Stop()
{
	m_bForceClose = true;
}

void FPathFinder::TraceParentOwnership()
{
	GridEntityPtr current = m_EndPtr;

	while (current != m_StartPtr)
	{
		FGridCoordinate offset(TRACE_RADIUS, TRACE_RADIUS);
		FGridCoordinate loc = (current->Location - offset) + m_StartLocation;
		FVector Location3D = UGridFunctions::GridToWorldLocation(loc);
		Location3D.Z = (float)current->Location.Z;

		m_TargetPath.Push(Location3D);
		current = current->Parent;
	}

	Algo::Reverse(m_TargetPath);
}

int FPathFinder::GetDistance(const GridEntityPtr begin, const GridEntityPtr end)
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

std::vector<GridEntityPtr> FPathFinder::GetTraversableNeighbors(GridEntityPtr Current)
{
	std::vector<GridEntityPtr> Results;
	const FGridCoordinate& loc = Current->Location;

	// Get Direct Neighbors First
	GridEntityPtr Top = GetElementAt(loc.X + 0, loc.Y + 1);
	GridEntityPtr Bottom = GetElementAt(loc.X + 0, loc.Y - 1);
	GridEntityPtr Left = GetElementAt(loc.X - 1, loc.Y + 0);
	GridEntityPtr Right = GetElementAt(loc.X + 1, loc.Y + 0);

	bool TopAdded = AddIfAvailable(Current, Top, Results);
	bool BottomAdded = AddIfAvailable(Current, Bottom, Results);
	bool LeftAdded = AddIfAvailable(Current, Left, Results);
	bool RightAdded = AddIfAvailable(Current, Right, Results);

	if (TopAdded && RightAdded)
	{
		GridEntityPtr TopRight = GetElementAt(loc.X + 1, loc.Y + 1);
		AddIfAvailable(Current, TopRight, Results);
	}

	if (TopAdded && LeftAdded)
	{
		GridEntityPtr TopLeft = GetElementAt(loc.X - 1, loc.Y + 1);
		AddIfAvailable(Current, TopLeft, Results);
	}

	if (BottomAdded && RightAdded)
	{
		GridEntityPtr BottomRight = GetElementAt(loc.X + 1, loc.Y - 1);
		AddIfAvailable(Current, BottomRight, Results);
	}

	if (BottomAdded && LeftAdded)
	{
		GridEntityPtr BottomLeft = GetElementAt(loc.X - 1, loc.Y - 1);
		AddIfAvailable(Current, BottomLeft, Results);
	}

	return Results;
}

FGridCoordinate FPathFinder::TracePlayArea()
{
	FGridCoordinate diff = m_EndLocation - m_StartLocation;

	m_GeneratedGridData.resize((TRACE_RADIUS * 2) + 1);
	for (int x = -TRACE_RADIUS; x <= TRACE_RADIUS; x++)
	{
		m_GeneratedGridData[x + TRACE_RADIUS].resize((TRACE_RADIUS * 2) + 1);
		for(int y = -TRACE_RADIUS; y <= TRACE_RADIUS; y++)
		{
			m_GeneratedGridData[x + TRACE_RADIUS][y + TRACE_RADIUS] = std::make_shared<GridEntry>();
			m_GeneratedGridData[x + TRACE_RADIUS][y + TRACE_RADIUS]->Location = FGridCoordinate(x + TRACE_RADIUS, y + TRACE_RADIUS, 0);
			m_GeneratedGridData[x + TRACE_RADIUS][y + TRACE_RADIUS]->Parent = nullptr;

			if (x == 0 && y == 0)
			{
				m_StartPtr = m_GeneratedGridData[x + TRACE_RADIUS][y + TRACE_RADIUS];
				m_StartPtr->Location.Z = m_StartLocation.Z;
			}
			else if (x == diff.X && y == diff.Y)
			{
				m_EndPtr = m_GeneratedGridData[x + TRACE_RADIUS][y + TRACE_RADIUS];
			}
		}
	}

	return FGridCoordinate((TRACE_RADIUS * 2) + 1, (TRACE_RADIUS * 2) + 1);
}

GridEntityPtr FPathFinder::GetElementAt(int x, int y)
{
	if (m_Bounds.X > x && x >= 0)
	{
		if (m_Bounds.Y > y && y >= 0)
		{
			return m_GeneratedGridData[x][y];
		}
	}

	return nullptr;
}

bool FPathFinder::AddIfAvailable(GridEntityPtr & Start, GridEntityPtr & End, std::vector<GridEntityPtr>& ValidNeighbors)
{
	if (End)
	{
		if (End->Location.Z == 0)
		{
			FGridCoordinate offset(TRACE_RADIUS, TRACE_RADIUS);
			FGridCoordinate loc = (End->Location - offset) + m_StartLocation;
			float StartHeight = Start->Location.Z;

			if (loc.X >= 0 && loc.Y >= 0)
			{
				FHitResult result;
				ETraceTypeQuery traceChannel = ETraceTypeQuery::TraceTypeQuery1;
				FVector s = UGridFunctions::GridToWorldLocation(FGridCoordinate(loc.X, loc.Y, StartHeight + (STEP_HEIGHT * 2.0f)));
				FVector f = UGridFunctions::GridToWorldLocation(FGridCoordinate(loc.X, loc.Y, StartHeight - (STEP_HEIGHT * 2.0f)));
				//FVector s = UGridFunctions::GridToWorldLocation(FGridCoordinate(loc.X, loc.Y, 10000));
				//FVector f = UGridFunctions::GridToWorldLocation(FGridCoordinate(loc.X, loc.Y, 0));
				
				bool traceComplete = false;
				FThreadSafeBool taskComplete = false;
				AsyncTask(ENamedThreads::GameThread, [&]() {
					// code to execute on game thread here
					if (m_World)
					{
						traceComplete = UBasicFunctions::TraceLine(s, f, m_World, result, traceChannel, nullptr, false);
						taskComplete = true;
					}
				});

				// Block until task in the main thread is done
				while (!taskComplete);

				if (traceComplete)
				{
					End->Location.Z = result.ImpactPoint.Z;
				}
				else
				{
					End->Location.Z = 0;
				}
			}
			else
			{
				End->Location.Z = -1;
			}

		}
		
		if (End->Location.Z != -1 && End->Location.Z != 0)
		{
			float heightDif = (float)abs((Start->Location.Z) - End->Location.Z);
			if (heightDif <= STEP_HEIGHT)
			{
				ValidNeighbors.push_back(End);
				return true;
			}
		}

	}

	return false;
}

void FPathFinder::EndThreadedTask()
{
	FString NewThreadName = "FPathFinder" + FString::FromInt(id);
	if (m_bForceClose)
	{
		// UE_LOG(LogNotice, Warning, TEXT("<%s>: Trace canceling..."), *NewThreadName);
	}

	Stop();
	m_Thread->WaitForCompletion();


	m_Callback.ExecuteIfBound(m_bTargetFound, m_TargetPath);
	// UE_LOG(LogNotice, Warning, TEXT("<%s>: Trace finished!"), *NewThreadName);
	delete this;
}

void FPathFinder::ThreadOnly_SubmitEndEvent()
{
	AsyncTask(ENamedThreads::GameThread, [this]() {
		// code to execute on game thread here
		EndThreadedTask();
	});
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
