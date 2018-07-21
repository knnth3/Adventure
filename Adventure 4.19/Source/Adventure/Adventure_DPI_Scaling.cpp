// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "Adventure_DPI_Scaling.h"



float UAdventure_DPI_Scaling::GetDPIScaleBasedOnSize(FIntPoint Size) const
{
	float BaseArea = X * Y;
	float CurrentArea = Size.X * Size.Y;

	return CurrentArea / BaseArea;
}