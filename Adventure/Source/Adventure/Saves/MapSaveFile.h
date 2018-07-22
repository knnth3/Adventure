// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include "Basics.h"
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MapSaveFile.generated.h"

/**
 * 
 */
UCLASS()
class ADVENTURE_API UMapSaveFile : public USaveGame
{
	GENERATED_BODY()
	
public:
	UMapSaveFile();

	UPROPERTY(BlueprintReadWrite)
	FString MapName;

	UPROPERTY(BlueprintReadWrite)
	FGridCoordinate MapSize;

	UPROPERTY(BlueprintReadWrite)
	TArray<struct FGAMEBUILDER_OBJECT> Objects;

};
