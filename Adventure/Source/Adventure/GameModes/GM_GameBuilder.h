// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include "Basics.h"
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GM_GameBuilder.generated.h"

/**
 * 
 */
UCLASS()
class ADVENTURE_API AGM_GameBuilder : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	UFUNCTION(BlueprintCallable, Category = "GameBuilder")
	bool IsNewMap()const;

	UFUNCTION(BlueprintCallable, Category = "GameBuilder")
	FGridCoordinate GetMapSize()const;

	UFUNCTION(BlueprintCallable, Category = "GameBuilder")
	FString GetMapName()const;

private:

	bool m_bNewMap;
	FString m_MapName;
	FGridCoordinate m_GridDimensions;
};
