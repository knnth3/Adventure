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

	void Initialize(FString MapName, int Rows, int Columns);
	virtual void HandleBeginPlay()override;

	UFUNCTION(BlueprintCallable, Category = "GameBuilder")
	FGridCoordinate GetMapSize()const;

	UFUNCTION(BlueprintCallable, Category = "GameBuilder")
	FString GetMapName()const;
	
protected:

	UPROPERTY(EditAnywhere, Category = "Spawnables")
	TSubclassOf<class AWorldGrid> GridClass;

private:

	FString m_MapName;
	int m_Rows, m_Columns;
	class AWorldGrid* m_WorldGrid;
};
