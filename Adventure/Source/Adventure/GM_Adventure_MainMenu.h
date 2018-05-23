// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GM_Adventure_MainMenu.generated.h"

/**
 * 
 */

UCLASS()
class ADVENTURE_API AGM_Adventure_MainMenu : public AGameModeBase
{
	GENERATED_BODY()
	
protected:
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

public:

	AGM_Adventure_MainMenu();

private:

	TSubclassOf<class AWorldGrid> GridClass;
	class AWorldGrid* WorldGrid;

};
