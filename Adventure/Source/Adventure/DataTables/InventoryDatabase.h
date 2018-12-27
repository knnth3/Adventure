// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include "Engine/DataTable.h"
#include "Character/HeldObject.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameFramework/Actor.h"
#include "InventoryDatabase.generated.h"


UENUM(BlueprintType)
enum class EItemCategory : uint8
{
	WEAPON,
	CONSUMABLE
};

USTRUCT(Blueprintable)
struct FConsumableInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FName Name = "";

	UPROPERTY(BlueprintReadWrite)
	int Weight = 0;

	UPROPERTY(BlueprintReadWrite)
	float Value = 0.f;

	UPROPERTY(BlueprintReadWrite)
	FName Description = "";

	UPROPERTY(BlueprintReadWrite)
	int HealthBonus = 0;

	UPROPERTY(BlueprintReadWrite)
	int VisualIndex = 0;
};

USTRUCT(Blueprintable)
struct FWeaponInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FName Name = "";

	UPROPERTY(BlueprintReadWrite)
	int Weight = 0;

	UPROPERTY(BlueprintReadWrite)
	int Damage = 0;

	UPROPERTY(BlueprintReadWrite)
	float Value = 0.f;

	UPROPERTY(BlueprintReadWrite)
	FName Description = "";

	UPROPERTY(BlueprintReadWrite)
	int VisualIndex = 0;
};

UCLASS()
class AConsumable : public AActor
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite)
	FName Name;

	UPROPERTY(BlueprintReadWrite)
	int Weight;

	UPROPERTY(BlueprintReadWrite)
	float Value;

	UPROPERTY(BlueprintReadWrite)
	FName Description;

	UPROPERTY(BlueprintReadWrite)
	int HealthBonus;

	UPROPERTY(BlueprintReadWrite)
	int VisualIndex;

	struct FItemVisualsRow* Visual = nullptr;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	FName GetVisualName()const;
};

UCLASS()
class AWeapon : public AActor
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite)
	FName Name;

	UPROPERTY(BlueprintReadWrite)
	int Weight;

	UPROPERTY(BlueprintReadWrite)
	int Damage;

	UPROPERTY(BlueprintReadWrite)
	float Value;

	UPROPERTY(BlueprintReadWrite)
	FName Description;

	UPROPERTY(BlueprintReadWrite)
	int VisualIndex;

	struct FItemVisualsRow* Visual = nullptr;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	FName GetVisualName()const;

};

// Lookup result row when using a UDataTable
USTRUCT(Blueprintable)
struct ADVENTURE_API FItemVisualsRow : public FTableRowBase
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

// Lookup result row when using a UDataTable
USTRUCT(Blueprintable)
struct ADVENTURE_API FWeaponDatabaseRow : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite)
	FName Name;

	UPROPERTY(BlueprintReadWrite)
	int Weight;

	UPROPERTY(BlueprintReadWrite)
	int Damage;

	UPROPERTY(BlueprintReadWrite)
	float Value;

	UPROPERTY(BlueprintReadWrite)
	FName Description;

	UPROPERTY(BlueprintReadWrite)
	int VisualIndex;
};

// Lookup result row when using a UDataTable
USTRUCT(Blueprintable)
struct ADVENTURE_API FConsumableDatabaseRow : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite)
	FName Name;

	UPROPERTY(BlueprintReadWrite)
	int Weight;

	UPROPERTY(BlueprintReadWrite)
	float Value;

	UPROPERTY(BlueprintReadWrite)
	FName Description;

	UPROPERTY(BlueprintReadWrite)
	int HealthBonus;

	UPROPERTY(BlueprintReadWrite)
	int VisualIndex;
};

// Separated data tables for lookup
static UDataTable* ItemVisualsLookUpTable = nullptr;

UCLASS()
class ADVENTURE_API UInventoryDatabase : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UInventoryDatabase();

	// Getters
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory Database")
	static AConsumable* GetConsumableFromDatabase(const FName& Name);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory Database")
	static AWeapon* GetWeaponFromDatabase(const FName& Name);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory Database")
	static TArray<FConsumableInfo> GetAllConsumablesInDatabase();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory Database")
	static TArray<FWeaponInfo> GetAllWeaponsInDatabase();

	// Setters
	UFUNCTION(BlueprintCallable, Category = "Inventory Database")
	static bool LoadConsumablesFromPath(FString Path);

	UFUNCTION(BlueprintCallable, Category = "Inventory Database")
	static bool LoadWeaponsFromPath(FString Path);

	UFUNCTION(BlueprintCallable, Category = "Inventory Database")
	static bool UpdateConsumableInDatabase(const FConsumableInfo& Consumable);

	UFUNCTION(BlueprintCallable, Category = "Inventory Database")
	static bool UpdateWeaponInDatabase(const FWeaponInfo& Weapon);

	UFUNCTION(BlueprintCallable, Category = "Inventory Database")
	static bool AddConsumableToDatabase(const FConsumableInfo& Consumable);

	UFUNCTION(BlueprintCallable, Category = "Inventory Database")
	static bool AddWeaponToDatabase(const FWeaponInfo& Weapon);

	UFUNCTION(BlueprintCallable, Category = "Inventory Database")
	static void RemoveConsumableFromDatabase(const FName& Name);

	UFUNCTION(BlueprintCallable, Category = "Inventory Database")
	static void RemoveWeaponFromDatabase(const FName& Name);

	UFUNCTION(BlueprintCallable, Category = "Inventory Database")
	static void ClearDatabase();

private:

	// Finds a weapon given an ItemIndex
	static FItemVisualsRow* GetInventoryItemVisual(uint8 ItemIndex);

	static TMap<FName, AConsumable*> m_Consumables;
	static TMap<FName, AWeapon*> m_Weapons;
};