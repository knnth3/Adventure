// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include "Basics.h"
#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GS_GameBuilder.generated.h"

/**
 * 
 */
UCLASS()
class ADVENTURE_API AGS_GameBuilder : public AGameStateBase
{
	GENERATED_BODY()
	
public: 
	virtual void HandleBeginPlay()override;

private:

	FString m_MapName;
	FGridCoordinate m_GridDimensions;
	class AWorldGrid* m_WorldGrid;
};
