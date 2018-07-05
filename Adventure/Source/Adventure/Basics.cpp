// Fill out your copyright notice in the Description page of Project Settings.

#include "Basics.h"

#define CM_TO_M_FACTOR 100
#define CM_TO_IN_FACTOR 2.54
#define IN_TO_YD_FACTOR 36
#define IN_TO_FT_FACTOR 12

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

FGridCoordinate::FGridCoordinate(CoordinatePair Location)
{
	X = Location.first;
	Y = Location.second;
}

bool FGridCoordinate::operator==(const FGridCoordinate & b)const
{
	return (X == b.X && Y == b.Y);
}

bool FGridCoordinate::operator!=(const FGridCoordinate & b)const
{
	return !operator==(b);
}

CoordinatePair FGridCoordinate::toPair()const
{
	return CoordinatePair(X, Y);
}

bool FGridCoordinate::IsZero() const
{
	return !(X && Y);
}

FVector UGridFunctions::GridToWorldLocation(const FGridCoordinate& Location)
{
	//Requres units to be in cm (ue4)
	float CELL_LENGTH, CELL_WIDTH;
	GetGridDimensions(CELL_LENGTH, CELL_WIDTH, UNITS::CENTIMETERS);

	return FVector(
		-(Location.X * CELL_LENGTH) - (CELL_LENGTH * 0.5f), 
		(Location.Y * CELL_WIDTH) + (CELL_WIDTH * 0.5f),
		0.0f
	);
}

FGridCoordinate UGridFunctions::WorldToGridLocation(const FVector& Location)
{
	//Requres units to be in cm (ue4)
	float CELL_LENGTH, CELL_WIDTH;
	GetGridDimensions(CELL_LENGTH, CELL_WIDTH, UNITS::CENTIMETERS);

	return FGridCoordinate(
		-FMath::TruncToInt(Location.X / CELL_LENGTH),
		FMath::TruncToInt(Location.Y / CELL_WIDTH)
	);
}

//Centimeters//////////////////////////////////////////////////////////////

float Conversions::Centimeters::ToInches(const float & Units)
{
	return Units / CM_TO_IN_FACTOR;
}

float Conversions::Centimeters::ToYards(const float & Units)
{
	float in = Conversions::Centimeters::ToInches(Units);

	return Conversions::Inches::ToYards(in);
}

float Conversions::Centimeters::ToMeters(const float & Units)
{
	return Units / CM_TO_M_FACTOR;
}

float Conversions::Centimeters::ToFeet(const float & Units)
{
	float in = Conversions::Centimeters::ToInches(Units);

	return Conversions::Inches::ToFeet(in);
}

//Inches////////////////////////////////////////////////////////////////////

float Conversions::Inches::ToFeet(const float & Units)
{
	return Units / IN_TO_FT_FACTOR;
}

float Conversions::Inches::ToCentimeters(const float & Units)
{
	return Units * CM_TO_IN_FACTOR;
}

float Conversions::Inches::ToYards(const float & Units)
{
	return Units / IN_TO_YD_FACTOR;
}

float Conversions::Inches::ToMeters(const float & Units)
{
	float cm = Conversions::Inches::ToCentimeters(Units);

	return Conversions::Centimeters::ToMeters(cm);
}

//Meters/////////////////////////////////////////////////////////////////////

float Conversions::Meters::ToCentimeters(const float & Units)
{
	return Units * CM_TO_M_FACTOR;
}

float Conversions::Meters::ToFeet(const float & Units)
{
	float cm = Conversions::Meters::ToCentimeters(Units);

	return Conversions::Centimeters::ToFeet(cm);
}

float Conversions::Meters::ToInches(const float & Units)
{
	float cm = Conversions::Meters::ToCentimeters(Units);

	return Conversions::Centimeters::ToInches(cm);
}

float Conversions::Meters::ToYards(const float & Units)
{
	float cm = Conversions::Meters::ToCentimeters(Units);

	return Conversions::Centimeters::ToYards(cm);
}

//Feet//////////////////////////////////////////////////////////////////

float Conversions::Feet::ToCentimeters(const float & Units)
{
	float in = Conversions::Feet::ToInches(Units);

	return Conversions::Inches::ToCentimeters(in);
}

float Conversions::Feet::ToInches(const float & Units)
{
	return Units * IN_TO_FT_FACTOR;
}

float Conversions::Feet::ToYards(const float & Units)
{
	float in = Conversions::Feet::ToInches(Units);

	return Conversions::Inches::ToYards(in);
}

float Conversions::Feet::ToMeters(const float & Units)
{
	float in = Conversions::Feet::ToInches(Units);

	return Conversions::Inches::ToMeters(in);
}
