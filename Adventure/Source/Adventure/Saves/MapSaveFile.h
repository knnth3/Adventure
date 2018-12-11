// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include "Basics.h"
#include "Grid/GridEntity.h"
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MapSaveFile.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct ADVENTURE_API FSAVE_OBJECT
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "GAMEBUILDER OBJECT")
	int ModelIndex;

	UPROPERTY(BlueprintReadWrite, Category = "GAMEBUILDER OBJECT")
	FString OwnerName;

	UPROPERTY(BlueprintReadWrite, Category = "GAMEBUILDER OBJECT")
	FGridCoordinate Location;

	UPROPERTY(BlueprintReadWrite, Category = "GAMEBUILDER OBJECT")
	GRID_OBJECT_TYPE Type;
};

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
	TArray<FSAVE_OBJECT> ObjectList;

	UPROPERTY(BlueprintReadWrite)
	TArray<uint8> HeightMap;

	UPROPERTY(BlueprintReadWrite)
	TArray<uint8> TextureMap;

};
