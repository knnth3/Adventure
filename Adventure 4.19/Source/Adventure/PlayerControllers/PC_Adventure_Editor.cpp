// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.
#include "PC_Adventure_Editor.h"

#include "Adventure.h"
#include "Grid/WorldGrid.h"


bool APC_Adventure_Editor::RequestSpawnInteractible(int Type, const FGridCoordinate& Location)
{
	bool Success = true;
	TActorIterator<AWorldGrid> GridItr(GetWorld());
	if (GridItr)
	{
		if (!GridItr->AddInteractable(Type, Location))
		{
			Success = false;
		}
	}
	return Success;
}

bool APC_Adventure_Editor::RequestDeleteObject(GAMEBUILDER_OBJECT_TYPE Type, const FGridCoordinate & Location)
{
	bool Success = false;
	TActorIterator<AWorldGrid> GridItr(GetWorld());
	if (GridItr)
	{
		switch (Type)
		{
		case GAMEBUILDER_OBJECT_TYPE::INTERACTABLE:
			Success = GridItr->RemoveInteractable(Location);
			break;
		case GAMEBUILDER_OBJECT_TYPE::SPAWN:
			Success = GridItr->RemoveSpawnLocation(Location);
			break;
		default:
			break;
		}
	}
	return Success;
}

bool APC_Adventure_Editor::RequestSetSpawnLocation(int Type, const FGridCoordinate& Location)
{
	bool Success = true;
	TActorIterator<AWorldGrid> GridItr(GetWorld());
	if (GridItr)
	{
		if (!GridItr->SetSpawnLocation(Type, Location))
		{
			Success = false;
		}
	}
	return Success;
}