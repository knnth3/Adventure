// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include <deque>
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
	virtual void HandleBeginPlay()override;

	UFUNCTION(BlueprintCallable, Category = "Turn-Based Mechanics")
	void SetActivePlayer(const int ID);

	UFUNCTION(BlueprintCallable, Category = "Turn-Based Mechanics")
	FString GetActivePlayerName() const;

protected:

	UPROPERTY(EditAnywhere, Category = "Spawnables")
	TSubclassOf<class AWorldGrid> GridClass;


private:

	UPROPERTY(Replicated)
	FString m_ActivePlayerName;

	bool m_bFreeRoamActive;
	int m_CurrentActivePlayer;
	class AWorldGrid* m_WorldGrid;
	std::deque<int> m_TurnSequence;
};
