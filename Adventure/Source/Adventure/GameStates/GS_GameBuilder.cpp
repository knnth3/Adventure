// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.
#include "GS_GameBuilder.h"

#include "GameModes/GM_GameBuilder.h"
#include "PlayerStates/PS_GameBuilder.h"
#include "Adventure.h"
#include "Grid/WorldGrid.h"
#include "DataTables/InventoryDatabase.h"

void AGS_GameBuilder::HandleBeginPlay()
{
	Super::HandleBeginPlay();

	if (HasAuthority())
	{
		bool bGenerateNewMap = false;
		AGM_GameBuilder* Gamemode = Cast<AGM_GameBuilder>(AuthorityGameMode);
		if (Gamemode)
		{
			if (Gamemode->IsNewMap())
			{
				FWeaponInfo Winfo;
				Winfo.Name = TEXT("Basic Sword");
				Winfo.Description = TEXT("Ye ol faithful");
				Winfo.VisualIndex = 1;
				UInventoryDatabase::AddWeaponToDatabase(Winfo);

				FConsumableInfo Cinfo;
				Cinfo.Name = TEXT("Cake");
				Cinfo.Description = TEXT("Delicious!");
				Cinfo.HealthBonus = 1;
				UInventoryDatabase::AddConsumableToDatabase(Cinfo);

				bGenerateNewMap = true;
			}
		}

		// Tell player states to load the area they need to display to the client
		for (const auto& ps : PlayerArray)
		{
			APS_GameBuilder* currentPS = Cast<APS_GameBuilder>(ps);
			if (currentPS)
			{
				if (bGenerateNewMap || !currentPS->LoadMap(Gamemode->GetMapName()))
				{
					currentPS->GenerateEmptyMap(Gamemode->GetMapName(), Gamemode->GetMapSize());
				}
			}
		}
	}
}
