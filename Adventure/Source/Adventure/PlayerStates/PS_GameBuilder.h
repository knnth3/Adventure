// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

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
