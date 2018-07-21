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
	AGS_Multiplayer();
	
	virtual void BeginPlay()override;
	virtual void OnRep_ReplicatedHasBegunPlay() override;
	AWorldGrid* GetWorldGrid();

private:
	TSubclassOf<class AWorldGrid> GridClass;
	class AWorldGrid* WorldGrid;
};
