// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <list>
#include <map>
#include <memory>

#include "CoreMinimal.h"
//#include "Engine\Source\Runtime\Core\Public\Containers\Array.h"
#include "Grid2D.generated.h"

typedef std::pair<int, int> coordinate;

struct DND_GAME_API GridCell
{
	GridCell()
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
		Location = coordinate(-1, -1);
	}

	GridCell(coordinate location) :
		GridCell()
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
		Location = location;
	}

	int F_Cost()
	{
		return H_Cost + G_Cost;
	}

	std::list<std::shared_ptr<GridCell>> GetEmptyNeighbors()
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

	//Pathfinding
	coordinate Location;
	bool Ocupied;
	int H_Cost;
	int G_Cost;
	std::shared_ptr<GridCell> Parent;

	//Direct
	std::shared_ptr<GridCell> Top;
	std::shared_ptr<GridCell> Left;
	std::shared_ptr<GridCell> Right;
	std::shared_ptr<GridCell> Bottom;

	//Diagonal
	std::shared_ptr<GridCell> TopLeft;
	std::shared_ptr<GridCell> BottomLeft;
	std::shared_ptr<GridCell> TopRight;
	std::shared_ptr<GridCell> BottomRight;
};

typedef std::shared_ptr<GridCell> GridCellPtr;

//Indexing starts at (1,1)

UCLASS()
class DND_GAME_API AGrid2D : public AActor
{

	GENERATED_BODY()

public:

	AGrid2D();
	AGrid2D(int rows, int columns);
	GridCellPtr At(int x, int y)const;
	int GetDistance(int x1, int y1, int x2, int y2)const;
	int GetDistance(const GridCellPtr& from, const GridCellPtr& to)const;
	bool FindPath(int x1, int y1, int x2, int y2, std::list<coordinate>& list);

	UFUNCTION(BlueprintCallable, Category = "Grid2D")
	void MakeGrid(int rows, int columns);

	UFUNCTION(BlueprintCallable, Category = "Grid2D")
	bool FindPath(int x1, int y1, int x2, int y2, TArray<FVector2D>& list);

	UFUNCTION(BlueprintCallable, Category = "Grid2D")
	void Clear();

private:

	int                               m_gridRows;
	int                               m_gridColumns;
	std::map<coordinate, GridCellPtr> m_gridIndex;
};

class DND_GAME_API AstarPathFind
{
public:
	bool FindPath(AGrid2D* grid, int x1, int y1, int x2, int y2, std::list<coordinate>& list);
	std::list<coordinate> TraceParentOwnership(const GridCellPtr& begin, const GridCellPtr& end);
};