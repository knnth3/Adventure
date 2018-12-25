// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include "Engine/DataTable.h"
#include "Character/HeldObject.h"
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InventoryDatabase.generated.h"

UENUM(BlueprintType)
enum class EItemCategory : uint8
{
	WEAPON,
	CONSUMABLE
};

// Lookup result row when using a UDataTable
USTRUCT(Blueprintable)
struct ADVENTURE_API FInventoryItem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	FName Name;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	EItemCategory Category;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	TSubclassOf<AHeldObject> Class = nullptr;
};

// Lookup result row when using a UDataTable
USTRUCT(Blueprintable)
struct ADVENTURE_API FInventoryDatabaseRow : public FTableRowBase
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "InventoryLookUp")
	FName Name;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "InventoryLookUp")
	EItemCategory Category;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "InventoryLookUp")
	TSubclassOf<AHeldObject> Class = nullptr;
};

// Separated data tables for lookup
static UDataTable* WeaponLookUpTable = nullptr;

// Separated data tables for lookup
static UDataTable* ConsumableLookUpTable = nullptr;

UCLASS()
class ADVENTURE_API UInventoryDatabase : public UObject
{
	GENERATED_BODY()

public:

	UInventoryDatabase();
	UInventoryDatabase(const FObjectInitializer& ObjectInitializer);

	// Finds a weapon given an ItemIndex
	static bool GetInventoryItem(EItemCategory Category, uint8 ItemIndex, FInventoryItem& Weapon);
};