// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

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

	UFUNCTION(BlueprintCallable, Category = "Multiplayer Gamemode")
	FGridCoordinate GetGridDimensions()const;
	
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

	UFUNCTION(BlueprintCallable, Category = "Multiplayer Gamemode")
	void SetGridDimensions2(const FGridCoordinate& Dimensions);

	UPROPERTY(EditAnywhere, Category = "Game Map Grid")
	TSubclassOf<class AWorldGrid> GridClass;

private:

	int PlayersConnected;
	FGridCoordinate GridDimensions; 
	class AWorldGrid* WorldGrid;
	TArray<FGAMEBUILDER_OBJECT> PendingObjects;
};
