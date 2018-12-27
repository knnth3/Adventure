// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "InventoryDatabase.h"
#include "UObject/ConstructorHelpers.h"
#include "Adventure.h"

TMap<FName, AConsumable*> UInventoryDatabase::m_Consumables = TMap<FName, AConsumable*>();
TMap<FName, AWeapon*> UInventoryDatabase::m_Weapons = TMap<FName, AWeapon*>();

UInventoryDatabase::UInventoryDatabase()
{
	static::ConstructorHelpers::FObjectFinder<UDataTable> WeaponTable(TEXT("/Game/Databases/DB_ItemVisuals"));
	if (WeaponTable.Object)
	{
		ItemVisualsLookUpTable = WeaponTable.Object;
	}
}

AConsumable* UInventoryDatabase::GetConsumableFromDatabase(const FName & Name)
{
	if (m_Consumables.Find(Name))
	{
		return m_Consumables[Name];
	}
	return nullptr;
}

AWeapon* UInventoryDatabase::GetWeaponFromDatabase(const FName & Name)
{
	if (m_Weapons.Find(Name))
	{
		return m_Weapons[Name];
	}
	return nullptr;
}

TArray<FConsumableInfo> UInventoryDatabase::GetAllConsumablesInDatabase()
{
	TArray<FConsumableInfo> consumables;
	for (const auto& consumable : m_Consumables)
	{
		FConsumableInfo newConsumable;
		newConsumable.Name = consumable.Value->Name;
		newConsumable.Description = consumable.Value->Description;
		newConsumable.Weight = consumable.Value->Weight;
		newConsumable.HealthBonus = consumable.Value->HealthBonus;
		newConsumable.VisualIndex = consumable.Value->VisualIndex;

		consumables.Push(newConsumable);
	}
	return consumables;
}

TArray<FWeaponInfo> UInventoryDatabase::GetAllWeaponsInDatabase()
{
	TArray<FWeaponInfo> weapons;
	for (const auto& weapon : m_Weapons)
	{
		FWeaponInfo newWeapon;
		newWeapon.Name = weapon.Value->Name;
		newWeapon.Description = weapon.Value->Description;
		newWeapon.Weight = weapon.Value->Weight;
		newWeapon.VisualIndex = weapon.Value->VisualIndex;

		weapons.Push(newWeapon);
	}
	return weapons;
}

bool UInventoryDatabase::LoadConsumablesFromPath(FString Path)
{
	return false;
}

bool UInventoryDatabase::LoadWeaponsFromPath(FString Path)
{
	return false;
}

bool UInventoryDatabase::UpdateConsumableInDatabase(const FConsumableInfo & Consumable)
{
	if (m_Consumables.Find(Consumable.Name))
	{
		auto& consumable = m_Consumables[Consumable.Name];
		consumable->Name = Consumable.Name;
		consumable->Description = Consumable.Description;
		consumable->HealthBonus = Consumable.HealthBonus;
		consumable->Weight = Consumable.Weight;

		if (consumable->VisualIndex != Consumable.VisualIndex)
		{
			consumable->VisualIndex = Consumable.VisualIndex;
			consumable->Visual = GetInventoryItemVisual(Consumable.VisualIndex);
		}

		UE_LOG(LogNotice, Log, TEXT("<InventoryDatabase>: Updated %s(Consumable)"), *Consumable.Name.ToString(), Consumable.VisualIndex);
		return true;
	}
	return false;
}

bool UInventoryDatabase::UpdateWeaponInDatabase(const FWeaponInfo & Weapon)
{
	if (m_Weapons.Find(Weapon.Name))
	{
		auto& weapon = m_Weapons[Weapon.Name];
		weapon->Name = Weapon.Name;
		weapon->Description = Weapon.Description;
		weapon->Weight = Weapon.Weight;

		if (weapon->VisualIndex != Weapon.VisualIndex)
		{
			weapon->VisualIndex = Weapon.VisualIndex;
			weapon->Visual = GetInventoryItemVisual(Weapon.VisualIndex);
		}

		UE_LOG(LogNotice, Log, TEXT("<InventoryDatabase>: Updated %s(Weapon)"), *Weapon.Name.ToString());
		return true;
	}
	return false;
}

bool UInventoryDatabase::AddConsumableToDatabase(const FConsumableInfo & Consumable)
{
	if (GEngine && GEngine->GameViewport)
	{
		UWorld* wworld = GEngine->GameViewport->GetWorld();
		if (wworld && !m_Consumables.Find(Consumable.Name))
		{
			AConsumable* newConsumable = wworld->SpawnActor<AConsumable>();
			newConsumable->Name = Consumable.Name;
			newConsumable->Description = Consumable.Description;
			newConsumable->HealthBonus = Consumable.HealthBonus;
			newConsumable->Weight = Consumable.Weight;
			newConsumable->Value = Consumable.Value;
			newConsumable->VisualIndex = Consumable.VisualIndex;
			newConsumable->Visual = GetInventoryItemVisual(Consumable.VisualIndex);

			m_Consumables.Emplace(Consumable.Name, newConsumable);

			UE_LOG(LogNotice, Log, TEXT("<InventoryDatabase>: Added %s(Consumable)"), *Consumable.Name.ToString());
			return true;
		}
	}
	return false;
}

bool UInventoryDatabase::AddWeaponToDatabase(const FWeaponInfo & Weapon)
{
	if (GEngine && GEngine->GameViewport)
	{
		UWorld* wworld = GEngine->GameViewport->GetWorld();
		if (wworld && !m_Weapons.Find(Weapon.Name))
		{
			AWeapon* newWeapon = wworld->SpawnActor<AWeapon>();
			newWeapon->Name = Weapon.Name;
			newWeapon->Description = Weapon.Description;
			newWeapon->Damage = Weapon.Damage;
			newWeapon->Value = Weapon.Value;
			newWeapon->Weight = Weapon.Weight;
			newWeapon->VisualIndex = Weapon.VisualIndex;
			newWeapon->Visual = GetInventoryItemVisual(Weapon.VisualIndex);

			m_Weapons.Emplace(Weapon.Name, newWeapon);

			UE_LOG(LogNotice, Log, TEXT("<InventoryDatabase>: Added %s(Weapon)"), *Weapon.Name.ToString());
			return true;
		}
	}
	return false;
}

void UInventoryDatabase::RemoveConsumableFromDatabase(const FName & Name)
{
	if (m_Consumables.Find(Name))
	{
		m_Consumables[Name]->Destroy();
		m_Consumables.Remove(Name);
	}
}

void UInventoryDatabase::RemoveWeaponFromDatabase(const FName & Name)
{
	if (m_Weapons.Find(Name))
	{
		m_Weapons[Name]->Destroy();
		m_Weapons.Remove(Name);
	}
}

void UInventoryDatabase::ClearDatabase()
{
	for (auto& consumable : m_Consumables)
	{
		consumable.Value->Destroy();
	}
	for (auto& weapon : m_Weapons)
	{
		weapon.Value->Destroy();
	}

	m_Consumables.Empty();
	m_Weapons.Empty();

	UE_LOG(LogNotice, Warning, TEXT("<InventoryDatabase>: Database Cleared!"));
}

FItemVisualsRow* UInventoryDatabase::GetInventoryItemVisual(uint8 ItemIndex)
{
	FItemVisualsRow* FoundRow = nullptr;
	FName RowName = *FString::FromInt((int)ItemIndex);
	FoundRow = ItemVisualsLookUpTable->FindRow<FItemVisualsRow>(RowName, FString("GENERAL"));

	return FoundRow;
}

FName AConsumable::GetVisualName() const
{
	if (Visual)
	{
		return Visual->Name;
	}
	return TEXT("None");
}

FName AWeapon::GetVisualName() const
{
	if (Visual)
	{
		return Visual->Name;
	}
	return TEXT("None");
}

