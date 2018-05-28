// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

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
	void GetGridDimensions(int& Rows, int& Columns)const;
	
protected:
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

private:

	int GridRows;
	int GridColumns;
	TSubclassOf<class AWorldGrid> GridClass;
	class AWorldGrid* WorldGrid;
};
