// Fill out your copyright notice in the Description page of Project Settings.

#include "Grid2D.h"
#define MAKE_CELL(x) GridCellPtr(new GridCell(x))
#define PRIORITY_QUEUE(T) std::priority_queue<std::shared_ptr<T>, std::vector<std::shared_ptr<T>>, shared_ptr_compare<T>>
#include <queue>
#include <functional>

AGrid2D::AGrid2D()
{
	m_gridRows = 0;
	m_gridColumns = 0;
}

AGrid2D::AGrid2D(int rows, int columns)
{
	MakeGrid(rows, columns);
}

GridCellPtr AGrid2D::At(int x, int y)const
{
	GridCellPtr cell = nullptr;
	bool xInRange = (x > 0) && (x <= m_gridRows);
	bool yInRange = (y > 0) && (y <= m_gridColumns);

	if (xInRange && yInRange)
	{
		coordinate index(x, y);
		cell = m_gridIndex.at(index);
	}

	return cell;
}

int AGrid2D::GetDistance(int x1, int y1, int x2, int y2)const
{
	int xDistance = abs(x1 - x2);
	int yDistance = abs(y1 - y2);

	if(xDistance > yDistance)
		return (14 * yDistance) + 10 * (xDistance - yDistance);

	return (14 * xDistance) + 10 * (yDistance - xDistance);
}


int AGrid2D::GetDistance(const GridCellPtr& from, const GridCellPtr& to)const
{
	return GetDistance(from->Location.first, from->Location.second, to->Location.first, to->Location.second);
}

bool AGrid2D::FindPath(int x1, int y1, int x2, int y2, std::list<coordinate>& list)
{
	AstarPathFind pathfinder;
	return pathfinder.FindPath(this, x1, y1, x2, y2, list);
}

void AGrid2D::MakeGrid(int rows, int columns)
{
	Clear();

	m_gridRows = rows;
	m_gridColumns = columns;

	for (int x = 1; x <= rows; x++)
	{
		for (int y = 1; y <= columns; y++)
		{
			//Create new cell
			coordinate position(x, y);
			m_gridIndex[position] = MAKE_CELL(position);

			//Link the top row if exists
			if (x > 1)
			{
				//Top & Bottom
				coordinate top(x - 1, y);
				m_gridIndex[position]->Top = m_gridIndex[top];
				m_gridIndex[top]->Bottom = m_gridIndex[position];

				//Top Right & Bottom Left
				if (y != columns)
				{
					coordinate topright(x - 1, y + 1);
					m_gridIndex[position]->TopRight = m_gridIndex[topright];
					m_gridIndex[topright]->BottomLeft = m_gridIndex[position];
				}
			}

			//Link the left column if exists
			if (y > 1)
			{
				//Left & Right
				coordinate left(x, y - 1);
				m_gridIndex[position]->Left = m_gridIndex[left];
				m_gridIndex[left]->Right = m_gridIndex[position];

				//Top Left & Bottom Right
				if (x > 1)
				{
					coordinate topleft(x - 1, y - 1);
					m_gridIndex[position]->TopLeft = m_gridIndex[topleft];
					m_gridIndex[topleft]->BottomRight = m_gridIndex[position];
				}
			}
		}
	}
}

bool AGrid2D::FindPath(int x1, int y1, int x2, int y2, TArray<FVector2D>& list)
{
	std::list<coordinate> coords;
	bool result = FindPath(-x1, y1, -x2, y2, coords);

	for (const auto& c : coords)
	{
		FVector2D converted;
		converted.X = -(float)c.first;
		converted.Y = (float)c.second;

		list.Push(converted);
	}

	return result;
}

void AGrid2D::Clear()
{
	m_gridRows = 0;
	m_gridColumns = 0;
	m_gridIndex.clear();
}

bool AstarPathFind::FindPath(AGrid2D * grid, int x1, int y1, int x2, int y2, std::list<coordinate>& list)
{
	if (!grid)
		return false;

	GridCellPtr start = grid->At(x1, y1);
	GridCellPtr end = grid->At(x2, y2);

	if (!start || !end)
		return false;

	PRIORITY_QUEUE(GridCell) openQueue;
	std::map<coordinate, GridCellPtr> openSet;
	std::map<coordinate, GridCellPtr> closedSet;

	openQueue.push(start);
	openSet[start->Location] = start;


	while (!openQueue.empty())
	{
		auto current = openQueue.top();
		openQueue.pop();
		openSet.erase(current->Location);

		closedSet[current->Location] = current;

		if (current == end)
		{
			//Finished
			list = TraceParentOwnership(start, end);
			return true;
		}

		for (const auto& neighbor : current->GetEmptyNeighbors())
		{
			if (closedSet.find(neighbor->Location) != closedSet.end())
			{
				continue;
			}

			int newG_Cost = current->G_Cost + grid->GetDistance(current, neighbor);

			bool bInOpenSet = false;
			if (openSet.find(neighbor->Location) != openSet.end())
			{
				bInOpenSet = true;
			}

			if (neighbor->Location == std::pair<int, int>(8, 9))
			{
				printf("");
			}

			if (newG_Cost < neighbor->G_Cost || !bInOpenSet)
			{
				neighbor->G_Cost = newG_Cost;
				neighbor->H_Cost = grid->GetDistance(neighbor, end);
				neighbor->Parent = current;

				if (!bInOpenSet)
				{
					openQueue.push(neighbor);
					openSet[neighbor->Location] = neighbor;
				}
			}
		}
	}

	return false;
}

std::list<coordinate> AstarPathFind::TraceParentOwnership(const GridCellPtr & begin, const GridCellPtr & end)
{
	std::list<coordinate> path;
	GridCellPtr current = end;
	
	while (current != begin)
	{
		path.push_back(current->Location);
		current = current->Parent;
	}

	path.reverse();

	return path;
}
