// Copyright 2019 Eric Marquez
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
// http ://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
