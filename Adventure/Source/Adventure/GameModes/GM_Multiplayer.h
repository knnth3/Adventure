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
	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	FGridCoordinate GetMapSize()const;

	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	FString GetMapName()const;

	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	void GetMapObjects(TArray<struct FGAMEBUILDER_OBJECT>& Objects)const;

	int GetHostID()const;

protected:
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void HandleSeamlessTravelPlayer(AController*& NewPlayer)override;

private:

	FString MapName;
	int Rows, Columns, HostID;
	TArray<struct FGAMEBUILDER_OBJECT> MapDecorations;
};
