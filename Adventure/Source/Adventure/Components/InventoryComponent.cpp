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

#include "InventoryComponent.h"
#include "StatisticsComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Adventure.h"


// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	m_bOverburdened = false;
	m_Stats = nullptr;
	m_SkeletalMesh = nullptr;
	m_RightHandObject = nullptr;
	m_LeftHandObject = nullptr;
	m_Gold = 0;
	m_Silver = 0;
	m_Copper = 0;
	m_CurrentWeight = 0;
	m_MaxWeight = 100;
	m_bOverburdened = false;
}

// Attaches the Statistics component to be able to make stat changes
void UInventoryComponent::AttachStatistics(UStatisticsComponent * Statistics)
{
	m_Stats = Statistics;
}

bool UInventoryComponent::GiveConsumable(FName Name, int Quantity)
{
	auto ConsumablePtr = UInventoryDatabase::GetConsumableFromDatabase(Name);
	if (ConsumablePtr)
	{
		if (AddWeight(ConsumablePtr->Weight * Quantity))
		{
			if (m_Consumables.Find(Name))
			{
				m_Consumables[Name] += Quantity;
			}
			else
			{
				m_Consumables.Emplace(Name, Quantity);
			}

			return true;
		}
	}

	UE_LOG(LogNotice, Error, TEXT("<InventoryComponent>: Failed to give/find consumable with name: %s"), *Name.ToString());
	return false;
}

bool UInventoryComponent::GiveWeapon(FName Name, int Quantity)
{
	auto WeaponPtr = UInventoryDatabase::GetWeaponFromDatabase(Name);
	if (WeaponPtr)
	{
		if (AddWeight(WeaponPtr->Weight * Quantity))
		{
			if (m_Weapons.Find(Name))
			{
				m_Weapons[Name] += Quantity;
			}
			else
			{
				m_Weapons.Emplace(Name, Quantity);
			}

			return true;
		}
	}

	UE_LOG(LogNotice, Error, TEXT("<InventoryComponent>: Failed to give/find weapon with name: %s"), *Name.ToString());
	return false;
}

void UInventoryComponent::GiveCurrency(int Gold, int Silver, int Copper)
{
	m_Gold += Gold;
	m_Silver += Silver;
	m_Copper += Copper;
}

bool UInventoryComponent::RemoveConsumable(const FName & Name, int Quantity)
{
	// Item is found
	if (ContainsObject(EItemCategory::CONSUMABLE, Name))
	{
		if (m_Consumables[Name] == 1)
		{
			m_Consumables.Remove(Name);
		}
		else
		{
			m_Consumables[Name]--;
		}

		auto ConsumablePtr = UInventoryDatabase::GetConsumableFromDatabase(Name);
		if(ConsumablePtr)
			AddWeight(ConsumablePtr->Weight * Quantity * -1);

		return true;
	}

	return false;
}

bool UInventoryComponent::RemoveWeapon(const FName & Name, int Quantity)
{
	// Item is found

	if (ContainsObject(EItemCategory::WEAPON, Name))
	{
		if (m_Weapons[Name] == 1)
		{
			m_Weapons.Remove(Name);
		}
		else
		{
			m_Weapons[Name]--;
		}

		auto WeaponPtr = UInventoryDatabase::GetWeaponFromDatabase(Name);
		if(WeaponPtr)
			AddWeight(WeaponPtr->Weight * Quantity * -1);

		return true;
	}

	return false;
}

void UInventoryComponent::RemoveCurrency(int Gold, int Silver, int Copper)
{
	m_Gold = FMath::Max(0, m_Gold - Gold);
	m_Silver = FMath::Max(0, m_Silver - Silver);
	m_Copper = FMath::Max(0, m_Copper - Copper);
}

// Equips object if found in inventory
bool UInventoryComponent::EquipWeapon(FName Name, bool bInMainHand)
{
	if (ContainsObject(EItemCategory::WEAPON, Name))
	{
		m_Weapons[Name]--;
		UnequipWeapon(bInMainHand);

		if (bInMainHand)
			m_MainHandObject = Name;
		else
			m_OffHandObject = Name;


		OnObjectEquiped(EItemCategory::WEAPON, Name, bInMainHand);

		return true;
	}

	return false;
}

// Unequips object and stores back into inventory
void UInventoryComponent::UnequipWeapon(bool bInMainHand)
{
	if (bInMainHand)
	{
		if (m_MainHandObject != "")
		{
			m_Weapons[m_MainHandObject]++;
		}

		m_MainHandObject = TEXT("");
	}
	else
	{
		if (m_OffHandObject != "")
		{
			m_Weapons[m_OffHandObject]++;
		}

		m_OffHandObject = TEXT("");
	}

	OnObjectUnequiped(bInMainHand);
}

// Gets the number of objects held
int UInventoryComponent::GetObjectCount(uint8 objectEnum) const
{
	return 0;
}

// Gets the max weight
int UInventoryComponent::GetCarryCapacity() const
{
	return m_MaxWeight;
}

// Gets the current weight
int UInventoryComponent::GetCurrentCarryWeight() const
{
	return m_CurrentWeight;
}

// Sets the carry weight capacity
void UInventoryComponent::SetCarryCapacity(int newCapacity)
{

	bool nowOverburdened = ((int)m_CurrentWeight > newCapacity);
	if (m_bOverburdened != nowOverburdened)
	{
		m_bOverburdened = nowOverburdened;
		m_MaxWeight = newCapacity;
		OnOverburdenedStateChanged();

	}
}

// Gets all objects in inventory
TArray<FName> UInventoryComponent::GetWeapons() const
{
	TArray<FName> Objects;
	for (const auto& object : m_Weapons)
	{
		Objects.Add(object.Key);
	}

	return Objects;
}

TArray<FName> UInventoryComponent::GetConsumables() const
{
	TArray<FName> Objects;
	for (const auto& object : m_Consumables)
	{
		Objects.Add(object.Key);
	}

	return Objects;
}

AHeldObject * UInventoryComponent::GetWeapon(bool rightHand) const
{
	if (rightHand)
		return m_RightHandObject;
	else
		return m_LeftHandObject;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (Owner)
	{

		TArray<UActorComponent*> SkeletalMesh = Owner->GetComponentsByTag(USkeletalMeshComponent::StaticClass(), SkeletalMeshTag);
		if (SkeletalMesh.Num() != 0)
		{
			m_SkeletalMesh = Cast<USkeletalMeshComponent>(SkeletalMesh[0]);
		}
		else
		{
			UE_LOG(LogNotice, Error, TEXT("<InventoryComponent>: Failed to find skeletal mesh with tag %s"), *SkeletalMeshTag.ToString());
		}
	}
}

void UInventoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (m_RightHandObject)
		m_RightHandObject->Destroy();
	if (m_LeftHandObject)
		m_LeftHandObject->Destroy();

	m_RightHandObject = nullptr;
	m_LeftHandObject = nullptr;
}

void UInventoryComponent::OnObjectEquiped(EItemCategory Category, const FName& Name, bool bInMainHand)
{
	if (m_SkeletalMesh)
	{

		auto visual = GetItemVisual(Category, Name);
		if (visual)
		{
			if (visual->Class)
			{
				FActorSpawnParameters params;
				params.Owner = GetOwner();
				params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				FTransform ObjectTransform(FRotator::ZeroRotator, FVector::ZeroVector);
				AHeldObject* HeldObject = Cast<AHeldObject>(GetWorld()->SpawnActor(visual->Class, &ObjectTransform, params));
				if (HeldObject)
				{
					FAttachmentTransformRules rules(
						EAttachmentRule::SnapToTarget, 
						EAttachmentRule::SnapToTarget,
						EAttachmentRule::SnapToTarget,
						true);

					// Attatching to right hand
					if ((bInMainHand && !bIsLeftHanded) || (!bInMainHand && bIsLeftHanded))
					{
						m_RightHandObject = HeldObject;
						m_RightHandObject->AttachToComponent(m_SkeletalMesh, rules, RHandSocketName);
						OnWeaponEquiped.Broadcast(true);

					}
					else // Attatching to left hand
					{
						m_LeftHandObject = HeldObject;
						m_LeftHandObject->AttachToComponent(m_SkeletalMesh, rules, LHandSocketName);
						OnWeaponEquiped.Broadcast(false);
					}

					// Change stance if in main hand
					if (bInMainHand && m_Stats)
					{
						m_Stats->SetWeaponStance(HeldObject->GetWeaponStance());
					}

				}
				else
				{
					UE_LOG(LogNotice, Warning, TEXT("<InventoryComponent>: Failed to spawn weapon."));
					UnequipWeapon(bInMainHand);
				}
			}
			else
			{
				UE_LOG(LogNotice, Warning, TEXT("<InventoryComponent>: Attempted to wield %s but no visual class was found."), *visual->Name.ToString());
				UnequipWeapon(bInMainHand);
			}
		}
		else
		{
			UE_LOG(LogNotice, Warning, TEXT("<InventoryComponent>: No Visual for %s was found. "), *Name.ToString());
			UnequipWeapon(bInMainHand);
		}
	}
}

void UInventoryComponent::OnObjectUnequiped(bool bInMainHand)
{
	// Detaching from right hand
	if ((bInMainHand && !bIsLeftHanded) || (!bInMainHand && bIsLeftHanded))
	{
		if (m_RightHandObject)
		{
			OnWeaponUnequiped.Broadcast(true);
			m_RightHandObject->Destroy();
			m_RightHandObject = nullptr;
		}

	}
	else // Detaching from left hand
	{
		if (m_LeftHandObject)
		{
			OnWeaponUnequiped.Broadcast(false);
			m_LeftHandObject->Destroy();
			m_LeftHandObject = nullptr;
		}
	}

	// Change stance if in main hand
	if (bInMainHand && m_Stats)
	{
		m_Stats->SetWeaponStance(WEAPON_TYPE::UNARMED);
	}
}

void UInventoryComponent::OnOverburdenedStateChanged()
{
}

// Searches through inventory to find a given object
bool UInventoryComponent::ContainsObject(EItemCategory Category, const FName Name) const
{
	bool result = false;
	switch (Category)
	{
	case EItemCategory::WEAPON:
		if (m_Weapons.Contains(Name))
			result = (bool)m_Weapons[Name];
		break;
	case EItemCategory::CONSUMABLE:
		if (m_Consumables.Contains(Name))
			result = (bool)m_Consumables[Name];
		break;
	default:
		break;
	}

	return result;
}

bool UInventoryComponent::AddWeight(int deltaWeight)
{
	bool ovrbrden = (m_MaxWeight) ? m_CurrentWeight + deltaWeight > m_MaxWeight : false;
	if (ovrbrden != m_bOverburdened)
		OnOverburdenedStateChanged();

	m_CurrentWeight += deltaWeight;
	return true;
}

FItemVisualsRow* UInventoryComponent::GetItemVisual(EItemCategory Category, const FName & Name)
{
	FItemVisualsRow* ItemVisual = nullptr;
	switch (Category)
	{
	case EItemCategory::WEAPON:
	{
		auto weapon = UInventoryDatabase::GetWeaponFromDatabase(Name);
		if (weapon)
		{
			ItemVisual = weapon->Visual;
		}
	}
	break;
	case EItemCategory::CONSUMABLE:
	{
		auto consumable = UInventoryDatabase::GetConsumableFromDatabase(Name);
		if (consumable)
		{
			ItemVisual = consumable->Visual;
		}
	}
	break;
	default:
		break;
	}

	return ItemVisual;
}
