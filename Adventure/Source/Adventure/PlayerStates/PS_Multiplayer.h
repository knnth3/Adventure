// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include <bitset>
#include "Basics.h"
#include "Saves/MapSaveFile.h"
#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "PS_Multiplayer.generated.h"

/**
 * 
 */

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

	// Server only function to load map from file
	bool ServerOnly_LoadMap(const FString& MapName);

	// Sets the map that the player state will use to query locations that will be displayed
	bool LoadMap(const FString& MapName);

	// Generates a new map with a given map size
	void GenerateEmptyMap(const FString& MapName, const FGridCoordinate& MapSize);

protected:

	// Sets the turn based player order
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn-Based Settings")
	TArray<int> InitiativePlayerOrder;

	UFUNCTION(BlueprintCallable, Category = "Player State")
	void UpdateDataTransfer(float DeltaTime);

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

	// Unique identifier
	UPROPERTY(Replicated)
	int m_GameID;

	// Turn based state
	UPROPERTY(ReplicatedUsing = OnStateChanged)
	TURN_BASED_STATE m_CurrentState;

	// Current active player
	UPROPERTY(Replicated)
	int m_CurrentPlayerActive;

	// Receive packet from server
	UFUNCTION(Client, Reliable)
	void Client_BeginMapDownload();

	// Retrieves the data in the client download buffer and deserializes it to an FMapLocation
	bool GetLocationFromDownloadBuffer(FMapLocation& Location);

	// Transfers location data to owning client so that it may generate a grid
	void GenerateGrid(const FMapLocation& Data);
};
