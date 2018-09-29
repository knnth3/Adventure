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

protected:

	UPROPERTY(EditAnywhere, Category = "Spawnables")
	TSubclassOf<class AWorldGrid> GridClass;


private:

	FString MapName;
	int Rows, Columns;
	class AWorldGrid* WorldGrid;
};
