// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include <map>
#include <string>
#include "basics.h"
#include "CoreMinimal.h"
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

	// Sets game session to in progress and loads the map
	void StartGame();

	// Set map to load
	UFUNCTION(BlueprintCallable, Category = "Lobby Gamemode")
	void SetMapToLoad(const FString& Name);

	// Get map to load
	UFUNCTION(BlueprintCallable, Category = "Lobby Gamemode")
	void GetMapToLoad(FString& Name)const;

protected:

	// Function called when a player has successfully logged in
	virtual void PostLogin(APlayerController* NewPlayer) override;

private:

	// Generates a playerID to be able to refrence separate instances
	int GeneratePlayerID();

	// Handles login attempt (ensure the function remains fast and simple or client will hang)
	void LoginConnectedPlayer(AController * Player);

	bool LoadMap(const FString & MapName);

	bool m_bMapHasBeenQueued;
	int m_PlayerIndexCount;
	FString m_CurrentMapName;
	FString m_HostUsername;
	FGridCoordinate m_GridDimensions;
	std::map<std::string, int> m_ConnnectedPlayers;
};
