// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <map>
#include "CoreMinimal.h"
#include "basics.h"
#include "Widgets/W_GameBuilderUI.h"
#include "GameFramework/GameModeBase.h"
#include "GM_Multiplayer.generated.h"


UENUM()
enum class GM_MULTIPLAYER_STATE : uint8
{
	ROAMING,
	BATTLE
};

/**
 * 
 */
UCLASS()
class ADVENTURE_API AGM_Multiplayer : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AGM_Multiplayer();

	UFUNCTION(BlueprintCallable, Category = "Multiplayer Gamemode")
	FGridCoordinate GetGridDimensions()const;

	UFUNCTION(BlueprintCallable, Category = "Multiplayer Gamemode")
	GM_MULTIPLAYER_STATE GetGameState()const;

	UFUNCTION(BlueprintCallable, Category = "Multiplayer Gamemode")
	void SetGameState(GM_MULTIPLAYER_STATE State);

	UFUNCTION(BlueprintCallable, Category = "Multiplayer Gamemode")
	bool IsPlayersTurn(const int PlayerID);

	void EndTurn(const int PlayerID);

protected:
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

	UFUNCTION(BlueprintNativeEvent, Category = "Multiplayer Gamemode")
	bool OnLoadMapRequest(const FString& SaveSlot);

	//Sends request to WorldGrid
	UFUNCTION(BlueprintCallable, Category = "Multiplayer Gamemode")
	bool RequestSpawnInteractible(int Type, const FGridCoordinate& Location);

	UFUNCTION(BlueprintCallable, Category = "Multiplayer Gamemode")
	bool RequestDeleteObject(GAMEBUILDER_OBJECT_TYPE Type, const FGridCoordinate& Location);

	UFUNCTION(BlueprintCallable, Category = "Multiplayer Gamemode")
	bool RequestSetSpawnLocation(int Type, const FGridCoordinate& Location);

	UFUNCTION(BlueprintCallable, Category = "Multiplayer Gamemode")
	void SetGridDimensions(const FGridCoordinate& Dimensions);

	UFUNCTION(BlueprintCallable, Category = "Multiplayer Gamemode")
	void AddObjectForPreInit(const FGAMEBUILDER_OBJECT& object);

	UPROPERTY(EditAnywhere, Category = "Game Map Grid")
	TSubclassOf<class AWorldGrid> GridClass;

private:

	int CurrentPlayersTurn;
	GM_MULTIPLAYER_STATE CurrentState;
	int PlayersConnected;
	FGridCoordinate GridDimensions; 
	class AWorldGrid* WorldGrid;
	TArray<FGAMEBUILDER_OBJECT> PendingObjects;
	std::map<int32, class AConnectedPlayer*> ConnectedPlayers;
};
