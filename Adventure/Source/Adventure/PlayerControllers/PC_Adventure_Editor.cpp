// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.
#include "PC_Adventure_Editor.h"

#include "Saves/MapSaveFile.h"
#include "Adventure.h"
#include "Grid/WorldGrid.h"


bool APC_Adventure_Editor::RequestSpawnBlockingObject(int Type, const FGridCoordinate& Location)
{
	bool Success = true;
	TActorIterator<AWorldGrid> GridItr(GetWorld());
	if (GridItr)
	{
		if (!GridItr->ServerOnly_AddBlockingObject(Type, Location))
		{
			Success = false;
		}
	}
	return Success;
}

bool APC_Adventure_Editor::RequestAddSpawnLocation(int Type, const FGridCoordinate& Location)
{
	return false;
}

bool APC_Adventure_Editor::RequestDeleteObject(GRID_OBJECT_TYPE Type, const FGridCoordinate & Location, int ID)
{
	bool Success = false;
	TActorIterator<AWorldGrid> GridItr(GetWorld());
	if (GridItr)
	{
		switch (Type)
		{
		case GRID_OBJECT_TYPE::INTERACTABLE:
			Success = GridItr->ServerOnly_RemoveBlockingObject(Location);
			break;
		case GRID_OBJECT_TYPE::PAWN:
			Success = GridItr->ServerOnly_RemovePawn(Location, ID);
			break;
		default:
			break;
		}
	}
	return Success;
}