// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

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
	AGM_GameBuilder(); 
	
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	
private:
	FName MapName;
	TSubclassOf<class AWorldGrid> GridClass;
	class AWorldGrid* WorldGrid;
};
