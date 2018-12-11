// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <map>
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

	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	FGridCoordinate GetMapSize()const;

	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	FString GetMapName()const;

	int GetHostID()const;

protected:
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController * NewPlayer) override;
	virtual void HandleSeamlessTravelPlayer(AController *& C) override;
	virtual void PostSeamlessTravel() override;

private:

	int GeneratePlayerID();
	void LoginConnectedPlayer(APlayerController *& Player);

	int m_PlayerIndexCount;
	FString m_CurrentMapName;
	FString m_HostUsername;
	FGridCoordinate m_GridDimensions;
	std::map<std::string, int> m_ConnnectedPlayers;
};
