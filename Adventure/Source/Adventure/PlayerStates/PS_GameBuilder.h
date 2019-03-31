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

#pragma once

#include "Basics.h"
#include "Saves/MapSaveFile.h"
#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PS_GameBuilder.generated.h"


UCLASS()
class ADVENTURE_API APS_GameBuilder : public APlayerState
{
	GENERATED_BODY()
	
public:
	// Sets the map that the player state will use to query locations that will be displayed
	bool LoadMap(const FString& MapName);

	// Generates a new map with a given map size
	void GenerateEmptyMap(const FString& MapName, const FGridCoordinate& MapSize);

private:

	// Transfers location data to owning client so that it may generate a grid (may be susceptible to too much data transfer crash)
	UFUNCTION(Client, Reliable)
	void Client_GenerateGrid(const FMapLocation& Data);

	// Tells client to create a new map
	UFUNCTION(Client, Reliable)
	void Client_GenerateEmptyGrid(const FGridCoordinate& MapSize);
};
