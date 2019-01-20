// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <map>
#include <string>
#include "CoreMinimal.h"
#include "basics.h"
#include "Widgets/W_GameBuilderUI.h"
#include "GameFramework/GameModeBase.h"
#include "GM_Multiplayer.generated.h"


/**
 * 
 */
UCLASS()
class ADVENTURE_API AGM_Multiplayer : public AGameModeBase
{
	GENERATED_BODY()
public:
	AGM_Multiplayer();

	// Gets the map size
	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	FGridCoordinate GetMapSize()const;

	// Gets the map name
	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	FString GetMapName()const;

	// Gets the PlayerID of the host
	int GetHostID()const;

protected:

	// Intializes game with default settings
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	// Callback for when player connects
	virtual void HandleStartingNewPlayer_Implementation(APlayerController * NewPlayer) override;

	// Callback for when player connects through seamless travel
	virtual void HandleSeamlessTravelPlayer(AController *& C) override;

	// Callback for when seamless travel is finished
	virtual void PostSeamlessTravel() override;

	// Called to login new player
	virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	// Callback for when seamless travel is finished
	virtual void PostLogin(APlayerController * NewPlayer) override;

	// Callback when a player logs out
	virtual void Logout(AController * Exiting) override;

private:

	// Generates a playerID to be able to refrence separate instances
	int GeneratePlayerID();

	// Handles login attempt (ensure the function remains fast and simple or client will hang)
	void LoginConnectedPlayer(AController * Player);

	bool m_MapDNE;
	int m_PlayerIndexCount;
	FString m_CurrentMapName;
	FString m_HostUsername;
	FGridCoordinate m_GridDimensions;
	std::map<std::string, int> m_ConnnectedPlayers;
};
