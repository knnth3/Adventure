// Copyright 2019 Eric Marquez
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
// http ://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
class ADVENTURE_API ULocationSave : public USaveGame
{
	GENERATED_BODY()

public:

	// Location data to be serialized
	UPROPERTY(BlueprintReadWrite)
	FMapLocation LocationData;
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

	UPROPERTY(BlueprintReadWrite)
	TArray<FString> Players;

	UPROPERTY(BlueprintReadWrite)
	TArray<FString> PlayerLocationNames;
};
