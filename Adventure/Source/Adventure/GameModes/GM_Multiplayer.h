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
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

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

private:

	FGridCoordinate GridDimensions;
	TSubclassOf<class AWorldGrid> GridClass;
	class AWorldGrid* WorldGrid;
	TArray<FGAMEBUILDER_OBJECT> PendingObjects;
};
