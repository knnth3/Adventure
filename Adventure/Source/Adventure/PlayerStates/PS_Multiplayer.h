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

#include <bitset>
#include "Basics.h"
#include "DownloadManager/DownloadManager.h"
#include "Saves/MapSaveFile.h"
#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PS_Multiplayer.generated.h"


#define TRANSFER_BITFIELD_SIZE sizeof(int) * 8 * 5

USTRUCT()
struct FLocationStats
{
	GENERATED_BODY()
public:

	UPROPERTY()
	int PackageSize;

	UPROPERTY()
	TArray<int> FinalizedBitField;
};

UENUM(BlueprintType)
enum class TURN_BASED_STATE : uint8
{
	FREE_ROAM,
	STANDBY,
	ACTIVE
};

UCLASS()
class ADVENTURE_API APS_Multiplayer : public APlayerState
{
	GENERATED_BODY()
	
public:
	APS_Multiplayer();

	// Sets the current gameID to refrence a player individually
	void ServerOnly_SetGameID(const int ID);

	// Sets the turn based player state
	UFUNCTION(BlueprintCallable, Category = "Player State")
	void ServerOnly_SetPlayerState(const TURN_BASED_STATE state);

	// Gets the current gameID to refrence a playre individually
	UFUNCTION(BlueprintCallable, Category = "Player State")
	int GetGameID() const;

	// Generates a new map with a given map size
	void GenerateEmptyMap(const FString& MapName, const FGridCoordinate& MapSize);

protected:

	// Sets the turn based player order
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn-Based Settings")
	TArray<int> InitiativePlayerOrder;

	// Override for server to change player order
	UFUNCTION(BlueprintCallable, Category = "Turn-Based Settings")
	void OverrideCurrentPlayersTurn(const int ID);

	// Gets the current turn based state
	UFUNCTION(BlueprintCallable, Category = "Turn-Based Settings")
	TURN_BASED_STATE GetCurrentState() const;

	// Callback to signal when turn based state has changed
	UFUNCTION(BlueprintImplementableEvent, Category = "Player State")
	void OnStateChanged();

private:

	// Retrieves the data in the client download buffer and deserializes it to an FMapLocation
	bool GetLocationFromDownloadBuffer(FMapLocation& Location);

	// Transfers location data to owning client so that it may generate a grid
	void GenerateGrid(const FMapLocation& Data);

	// Unique identifier
	UPROPERTY(Replicated)
	int m_GameID;

	// Turn based state
	UPROPERTY(ReplicatedUsing = OnStateChanged)
	TURN_BASED_STATE m_CurrentState;

	// Current active player
	UPROPERTY(Replicated)
	int m_CurrentPlayerActive;
};
