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

#include "PS_GameBuilder.h"
#include "Grid/WorldGrid.h"

bool APS_GameBuilder::LoadMap(const FString & MapName)
{
	TActorIterator<AWorldGrid> WorldGrid(GetWorld());
	if (WorldGrid)
	{
		WorldGrid->ServerOnly_SetMapName(MapName);
	}

	FString path = FString::Printf(TEXT("%sMaps/%s.map"), *FPaths::ProjectUserDir(), *MapName);
	UMapSaveFile* Save = Cast<UMapSaveFile>(UBasicFunctions::LoadSaveGameEx(path));
	if (Save)
	{
		FString CurrentLocation;
		for (int x = 0; x < Save->Players.Num(); x++)
		{
			// User found
			if (Save->Players[x] == GetPlayerName())
			{
				CurrentLocation = Save->PlayerLocationNames[x];
				break;
			}
		}

		if (CurrentLocation.IsEmpty())
		{
			CurrentLocation = Save->ActiveLocation;
		}

		for (const auto& loc : Save->Locations)
		{
			if (loc.Name == CurrentLocation)
			{
				Client_GenerateGrid(loc);
				break;
			}
		}

		return true;
	}

	return false;
}

void APS_GameBuilder::GenerateEmptyMap(const FString& MapName, const FGridCoordinate & MapSize)
{
	TActorIterator<AWorldGrid> WorldGrid(GetWorld());
	if (WorldGrid)
	{
		WorldGrid->ServerOnly_SetMapName(MapName);
		Client_GenerateEmptyGrid(MapSize);
	}
}

void APS_GameBuilder::Client_GenerateGrid_Implementation(const FMapLocation & Data)
{
	UE_LOG(LogNotice, Warning, TEXT("<PlayerState>: Map loaded: %s"), *Data.Name);
	TActorIterator<AWorldGrid> WorldGrid(GetWorld());
	if (WorldGrid)
	{
		WorldGrid->BuildLocation(Data);
	}
}

void APS_GameBuilder::Client_GenerateEmptyGrid_Implementation(const FGridCoordinate & MapSize)
{
	UE_LOG(LogNotice, Warning, TEXT("<PlayerState>: Generating empty map..."));
	TActorIterator<AWorldGrid> WorldGrid(GetWorld());
	if (WorldGrid)
	{
		WorldGrid->GenerateEmptyLocation(MapSize);
	}
}
