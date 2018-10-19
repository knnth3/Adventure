// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.
#include "GS_GameBuilder.h"

#include "GameModes/GM_GameBuilder.h"
#include "Adventure.h"
#include "Grid/WorldGrid.h"

void AGS_GameBuilder::HandleBeginPlay()
{
	Super::HandleBeginPlay();

	if (HasAuthority())
	{
		AGM_GameBuilder* Gamemode = Cast<AGM_GameBuilder>(AuthorityGameMode);
		TActorIterator<AWorldGrid> WorldGridItr(GetWorld());
		if (Gamemode && WorldGridItr)
		{
			bool loaded = !Gamemode->IsNewMap();
			if (loaded)
			{
				if (!WorldGridItr->ServerOnly_LoadGrid(Gamemode->GetMapName()))
				{
					UE_LOG(LogNotice, Warning, TEXT("<GameState Setup>: Failed to load grid"));
					loaded = false;
				}
			}

			if (!loaded)
			{
				if (!WorldGridItr->ServerOnly_GenerateGrid(Gamemode->GetMapName(), Gamemode->GetMapSize()))
				{
					UE_LOG(LogNotice, Warning, TEXT("<GameState Setup>: Failed to initialize grid"));
				}
			}
		}
	}
}
