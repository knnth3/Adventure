// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Grid/WorldGrid.h"
#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GS_Multiplayer.generated.h"

/**
 * 
 */

UCLASS()
class ADVENTURE_API AGS_Multiplayer : public AGameStateBase
{
	GENERATED_BODY()
	
public:

	virtual void HandleBeginPlay()override;

	//UFUNCTION(BlueprintNativeEvent, Category = "Multiplayer Gamemode")
	//bool OnLoadMapRequest(const FString& SaveSlot);

	////Sends request to WorldGrid
	//UFUNCTION(BlueprintCallable, Category = "Multiplayer Gamemode")
	//bool RequestAddBlockingSpace(int Type, const FGridCoordinate& Location);

	//UFUNCTION(BlueprintCallable, Category = "Multiplayer Gamemode")
	//bool RequestDeleteObject(GAMEBUILDER_OBJECT_TYPE Type, const FGridCoordinate& Location);

	//UFUNCTION(BlueprintCallable, Category = "Multiplayer Gamemode")
	//bool RequestSetSpawnLocation(int Type, const FGridCoordinate& Location);

	//UFUNCTION(BlueprintCallable, Category = "Multiplayer Gamemode")
	//void SetGridDimensions(const FGridCoordinate& Dimensions);

	//UFUNCTION(BlueprintCallable, Category = "Multiplayer Gamemode")
	//void AddObjectForPreInit(const FGAMEBUILDER_OBJECT& object);

protected:

	UPROPERTY(EditAnywhere, Category = "Spawnables")
	TSubclassOf<class AWorldGrid> GridClass;


private:

	FString MapName;
	int Rows, Columns;
	class AWorldGrid* WorldGrid;
};
