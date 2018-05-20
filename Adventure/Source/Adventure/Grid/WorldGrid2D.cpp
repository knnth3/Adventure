// Fill out your copyright notice in the Description page of Project Settings.
#include "WorldGrid2D.h"

#include <queue>
#include <functional>
#include "Adventure.h"

#define MAKE_CELL(x) GridCellPtr(new GridCell(x))
#define PRIORITY_QUEUE(T) std::priority_queue<std::shared_ptr<T>, std::vector<std::shared_ptr<T>>, shared_ptr_compare<T>>


AWorldGrid2D::AWorldGrid2D()
{
	m_gridRows = 0;
	m_gridColumns = 0;
}

AWorldGrid2D::AWorldGrid2D(int Rows, int Columns)
{
	MakeGrid(Rows, Columns);
}

GridCellPtr AWorldGrid2D::At(FGridCoordinate Location) const
{
	GridCellPtr cell = nullptr;

	if (m_grid.size() > Location.X && Location.X >= 0)
	{
		if (m_grid[Location.X].size() > Location.Y && Location.Y >= 0)
		{
			cell = m_grid[Location.X][Location.Y];
		}
	}

	return cell;
}

int AWorldGrid2D::GetDistance(FGridCoordinate Begin, FGridCoordinate End)const
{
	int xDistance = abs(Begin.X - End.X);
	int yDistance = abs(Begin.Y - End.Y);

	if (xDistance > yDistance)
		return (14 * yDistance) + 10 * (xDistance - yDistance);

	return (14 * xDistance) + 10 * (yDistance - xDistance);
}

int AWorldGrid2D::GetDistance(const GridCellPtr& Begin, const GridCellPtr& End)const
{
	return GetDistance(Begin->Location, End->Location);
}

void AWorldGrid2D::MakeGrid(int Rows, int Columns)
{
	Clear();

	m_gridRows = Rows;
	m_gridColumns = Columns;

	for (int x = 0; x < Rows; x++)
	{
		//make a new row for this column
		m_grid.push_back(std::vector<GridCellPtr>());
		for (int y = 0; y < Columns; y++)
		{
			//Create new cell
			auto Current = MAKE_CELL(FGridCoordinate(x, y));
			m_grid[x].push_back(Current);

			//Link the top row if exists
			if (x > 1)
			{
				//Top & Bottom
				auto Top = At(FGridCoordinate(x - 1, y));
				if (Top)
				{
					Current->Top = Top;
					Top->Bottom = Current;
				}

				//Top Right & Bottom Left
				if (y != Columns)
				{
					auto TopRight = At(FGridCoordinate(x - 1, y + 1));
					if (TopRight)
					{
						Current->TopRight = TopRight;
						TopRight->BottomLeft = Current;
					}
				}
			}

			//Link the left column if exists
			if (y > 1)
			{
				//Left & Right
				auto Left = At(FGridCoordinate(x, y - 1));
				if (Left)
				{
					Current->Left = Left;
					Left->Right = Current;
				}

				//Top Left & Bottom Right
				if (x > 1)
				{
					auto TopLeft = At(FGridCoordinate(x - 1, y - 1));
					if (TopLeft)
					{
						Current->TopLeft = TopLeft;
						TopLeft->BottomRight = Current;
					}
				}
			}
		}
	}
}

bool AWorldGrid2D::FindPath(FGridCoordinate Begin, FGridCoordinate End, TArray<FGridCoordinate>& List)
{
	AstarPathFind pathfinder;

	std::list<FGridCoordinate> path;
	bool result = pathfinder.FindPath(this, Begin, End, path);

	for (const auto& c : path)
	{
		List.Push(c);
	}

	return result;
}

void AWorldGrid2D::Clear()
{
	m_gridRows = 0;
	m_gridColumns = 0;
	m_grid.clear();
}

void AWorldGrid2D::SetSpawnLocations(const TArray<FGridCoordinate>& Locations)
{
	for (const auto& item : Locations)
	{
		GridCellPtr cell = At(item);
		if (cell)
		{
			m_spawnLocations.push_back(cell->Location);
		}
	}
}

bool AWorldGrid2D::GetOpenSpawnLocation(FGridCoordinate& GridLocation)
{
	for (const auto& location : m_spawnLocations)
	{
		GridCellPtr cell = At(location);
		if (cell && !cell->Ocupied)
		{
			GridLocation.X = -location.X;
			GridLocation.Y = location.Y;
			cell->Ocupied = true;
			return true;
		}
	}
	return false;
}

void AWorldGrid2D::RemoveSpawnLocation(FGridCoordinate Location)
{
	for (int index = 0; index < m_spawnLocations.size(); index++)
	{
		if (m_spawnLocations[index] == Location)
		{
			//Order of spawn locations do not matter
			int lastIndex = m_spawnLocations.size() - 1;
			if (index < lastIndex)
			{
				m_spawnLocations[index] = m_spawnLocations[lastIndex];
				m_spawnLocations.pop_back();
			}
			else
			{
				m_spawnLocations.pop_back();
			}
		}
	}
}

void AWorldGrid2D::SetOccupied(FGridCoordinate Location)
{
	GridCellPtr cell = At(Location);

	if (cell)
		cell->Ocupied = true;
}

bool AWorldGrid2D::IsOccupied(FGridCoordinate Location)const
{
	GridCellPtr cell = At(Location);

	if (cell)
		return cell->Ocupied;

	return false;
}

bool AstarPathFind::FindPath(AWorldGrid2D * Grid, FGridCoordinate Begin, FGridCoordinate End, std::list<FGridCoordinate>& List)
{
	if (!Grid)
		return false;

	GridCellPtr start = Grid->At(Begin);
	GridCellPtr end = Grid->At(End);

	if (!start || !end)
		return false;

	PRIORITY_QUEUE(GridCell) OpenQueue;
	std::map<coordinate, GridCellPtr> OpenSet;
	std::map<coordinate, GridCellPtr> ClosedSet;

	OpenQueue.push(start);
	OpenSet[GetCoordinate(start->Location)] = start;


	while (!OpenQueue.empty())
	{
		auto Current = OpenQueue.top();
		coordinate CurrentCoord = GetCoordinate(Current->Location);

		OpenQueue.pop();
		OpenSet.erase(CurrentCoord);

		ClosedSet[CurrentCoord] = Current;

		if (Current == end)
		{
			//Finished
			List = TraceParentOwnership(start, end);
			return true;
		}

		for (const auto& neighbor : Current->GetEmptyNeighbors())
		{
			coordinate NeighborCoord = GetCoordinate(neighbor->Location);

			if (ClosedSet.find(NeighborCoord) != ClosedSet.end())
			{
				continue;
			}

			int newG_Cost = Current->G_Cost + Grid->GetDistance(Current, neighbor);

			bool bInOpenSet = false;
			if (OpenSet.find(NeighborCoord) != OpenSet.end())
			{
				bInOpenSet = true;
			}

			if (newG_Cost < neighbor->G_Cost || !bInOpenSet)
			{
				neighbor->G_Cost = newG_Cost;
				neighbor->H_Cost = Grid->GetDistance(neighbor, end);
				neighbor->Parent = Current;

				if (!bInOpenSet)
				{
					OpenQueue.push(neighbor);
					OpenSet[NeighborCoord] = neighbor;
				}
			}
		}
	}

	return false;
}

std::list<FGridCoordinate> AstarPathFind::TraceParentOwnership(const GridCellPtr & Begin, const GridCellPtr & End)
{
	std::list<FGridCoordinate> path;
	GridCellPtr current = End;

	while (current != Begin)
	{
		path.push_back(current->Location);
		current = current->Parent;
	}

	path.reverse();

	return path;
}

coordinate AstarPathFind::GetCoordinate(const FGridCoordinate & Location) const
{
	return coordinate(Location.X, Location.Y);
}

GridCell::GridCell()
{
	Ocupied = false;
	H_Cost = 0;
	G_Cost = 0;
	Top = nullptr;
	Left = nullptr;
	Right = nullptr;
	Bottom = nullptr;
	Parent = nullptr;
	TopLeft = nullptr;
	BottomLeft = nullptr;
	TopRight = nullptr;
	BottomRight = nullptr;
	Location = FGridCoordinate(-1, -1);
}

GridCell::GridCell(FGridCoordinate Location):
	GridCell()
{
	this->Location = Location;
}

int GridCell::F_Cost() const
{
	return H_Cost + G_Cost;
}

bool GridCell::operator<(const GridCell & b)
{
	if (F_Cost() == b.F_Cost())
	{
		return H_Cost <= b.H_Cost;
	}

	return F_Cost() < b.F_Cost();
}

std::list<std::shared_ptr<GridCell>> GridCell::GetEmptyNeighbors()
{
	std::list<std::shared_ptr<GridCell>> list;
	// 8 total possible neighbors

	if (Top && !Top->Ocupied)
	{
		list.push_back(Top);
	}

	if (Bottom && !Bottom->Ocupied)
	{
		list.push_back(Bottom);
	}

	if (Left && !Left->Ocupied)
	{
		list.push_back(Left);
	}

	if (Right && !Right->Ocupied)
	{
		list.push_back(Right);
	}

	if (TopLeft && !TopLeft->Ocupied)
	{
		list.push_back(TopLeft);
	}

	if (TopRight && !TopRight->Ocupied)
	{
		list.push_back(TopRight);
	}

	if (BottomLeft && !BottomLeft->Ocupied)
	{
		list.push_back(BottomLeft);
	}

	if (BottomRight && !BottomRight->Ocupied)
	{
		list.push_back(BottomRight);
	}

	return list;
}
