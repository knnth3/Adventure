// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "InventoryDatabase.h"
#include "UObject/ConstructorHelpers.h"

UInventoryDatabase::UInventoryDatabase()
{
	static::ConstructorHelpers::FObjectFinder<UDataTable> WeaponTable(TEXT("/Game/Databases/DB_Weapons"));
	if (WeaponTable.Object)
	{
		WeaponLookUpTable = WeaponTable.Object;
	}

	static::ConstructorHelpers::FObjectFinder<UDataTable> ConsumableTable(TEXT("/Game/Databases/DB_Consumables"));
	if (ConsumableTable.Object)
	{
		ConsumableLookUpTable = ConsumableTable.Object;
	}
}

UInventoryDatabase::UInventoryDatabase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	static::ConstructorHelpers::FObjectFinder<UDataTable> WeaponTable(TEXT("/Game/Databases/DB_Weapons"));
	if (WeaponTable.Object)
	{
		WeaponLookUpTable = WeaponTable.Object;
	}

	static::ConstructorHelpers::FObjectFinder<UDataTable> ConsumableTable(TEXT("/Game/Databases/DB_Consumables"));
	if (ConsumableTable.Object)
	{
		ConsumableLookUpTable = ConsumableTable.Object;
	}

}

bool UInventoryDatabase::GetInventoryItem(EItemCategory Category, uint8 ItemIndex, FInventoryItem & Weapon)
{
	FInventoryDatabaseRow* FoundRow = nullptr;
	FName RowName = *FString::FromInt((int)ItemIndex);

	switch (Category)
	{
	case EItemCategory::WEAPON:
		FoundRow = WeaponLookUpTable->FindRow<FInventoryDatabaseRow>(RowName, FString("GENERAL"));
		break;
	case EItemCategory::CONSUMABLE:
		FoundRow = ConsumableLookUpTable->FindRow<FInventoryDatabaseRow>(RowName, FString("GENERAL"));
		break;
	default:
		break;
	}

	if (FoundRow)
	{
		Weapon.Category = FoundRow->Category;
		Weapon.Class = FoundRow->Class;
		Weapon.Name = FoundRow->Name;
	}

	return (bool)FoundRow;
}
