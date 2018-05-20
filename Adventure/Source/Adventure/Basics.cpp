// Fill out your copyright notice in the Description page of Project Settings.

#include "Basics.h"

FGridCoordinate::FGridCoordinate()
{
	X = 0;
	Y = 0;
}

FGridCoordinate::FGridCoordinate(int32 x, int32 y)
{
	X = x;
	Y = y;
}

FGridCoordinate::FGridCoordinate(FVector Location3D)
{
	FGridCoordinate temp = UGridFunctions::WorldToGridLocation(Location3D);
	X = temp.X;
	Y = temp.Y;
}

bool FGridCoordinate::operator==(const FGridCoordinate & b)
{
	return (X == b.X && Y == b.Y);
}

FVector UGridFunctions::GridToWorldLocation(const FGridCoordinate& Location)
{
	return FVector(
		(Location.X * CELL_LENGTH) - (CELL_LENGTH * 0.5f), 
		(Location.Y * CELL_WIDTH) + (CELL_WIDTH * 0.5f),
		0.0f
	);
}

FGridCoordinate UGridFunctions::WorldToGridLocation(const FVector& Location)
{
	return FGridCoordinate(
		FMath::TruncToInt(Location.X / CELL_LENGTH),
		FMath::TruncToInt(Location.Y / CELL_WIDTH)
	);
}