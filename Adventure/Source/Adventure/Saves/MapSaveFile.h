// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include "Basics.h"
#include "DataTables/InventoryDatabase.h"
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MapSaveFile.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct ADVENTURE_API FMapLocation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString Name;

	UPROPERTY(BlueprintReadWrite)
	FGridCoordinate Size;

	UPROPERTY(BlueprintReadWrite)
	TArray<uint8> HeightMap;

	UPROPERTY(BlueprintReadWrite)
	TArray<uint8> TextureMap;

	UPROPERTY(BlueprintReadWrite)
	TArray<uint8> Objects;

	UPROPERTY(BlueprintReadWrite)
	TArray<FTransform> ObjectTransforms;
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
	FString ActiveLocation;

	UPROPERTY(BlueprintReadWrite)
	TArray<FMapLocation> Locations;

	UPROPERTY(BlueprintReadWrite)
	TArray<FConsumableInfo> Consumables;

	UPROPERTY(BlueprintReadWrite)
	TArray<FWeaponInfo> Weapons;

};
