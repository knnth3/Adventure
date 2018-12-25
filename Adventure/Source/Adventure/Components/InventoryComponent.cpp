// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

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
void UInventoryComponent::AttatchStatistics(UStatisticsComponent * Statistics)
{
	m_Stats = Statistics;
}

bool UInventoryComponent::AddConsumable(const FConsumable & ConsumableInfo)
{
	// Item is found
	if (AddWeight(ConsumableInfo.Weight * ConsumableInfo.Quantity))
	{
		if (ContainsObject(EItemCategory::CONSUMABLE, ConsumableInfo.Name))
		{
			m_Consumables[ConsumableInfo.Name].Quantity += ConsumableInfo.Quantity;
			return true;
		}
		else
		{
			m_Consumables.Emplace(ConsumableInfo.Name, ConsumableInfo);
			return true;
		}
	}
	return false;
}

bool UInventoryComponent::AddWeapon(const FWeapon & WeaponInfo)
{
	// Item is found
	if (AddWeight(WeaponInfo.Weight * WeaponInfo.Quantity))
	{
		if (ContainsObject(EItemCategory::WEAPON, WeaponInfo.Name))
		{
			m_Weapons[WeaponInfo.Name].Quantity += WeaponInfo.Quantity;
			return true;
		}
		else
		{
			m_Weapons.Emplace(WeaponInfo.Name, WeaponInfo);
			return true;
		}
	}
	return false;
}

void UInventoryComponent::AddCurrency(int Gold, int Silver, int Copper)
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
		if (m_Consumables[Name].Quantity == 1)
		{
			m_Consumables.Remove(Name);
		}
		else
		{
			m_Consumables[Name].Quantity--;
		}

		AddWeight(m_Consumables[Name].Weight * Quantity * -1);
		return true;
	}

	return false;
}

bool UInventoryComponent::RemoveWeapon(const FName & Name, int Quantity)
{
	// Item is found
	if (ContainsObject(EItemCategory::WEAPON, Name))
	{
		if (m_Weapons[Name].Quantity == 1)
		{
			m_Weapons.Remove(Name);
		}
		else
		{
			m_Weapons[Name].Quantity--;
		}

		AddWeight(m_Weapons[Name].Weight * Quantity * -1);
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
		m_Weapons[Name].Quantity--;
		UnequipWeapon(bInMainHand);

		if (bInMainHand)
			m_MainHandObject = Name;
		else
			m_OffHandObject = Name;


		OnObjectEquiped(EItemCategory::WEAPON, m_Weapons[Name].VisualIndex, bInMainHand);

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
			m_Weapons[m_MainHandObject].Quantity++;
		}

		m_MainHandObject = TEXT("");
	}
	else
	{
		if (m_OffHandObject != "")
		{
			m_Weapons[m_OffHandObject].Quantity++;
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
TArray<FWeapon> UInventoryComponent::GetWeapons() const
{
	TArray<FWeapon> Objects;
	for (const auto& object : m_Weapons)
	{
		Objects.Add(object.Value);
	}

	return Objects;
}

TArray<FConsumable> UInventoryComponent::GetConsumables() const
{
	TArray<FConsumable> Objects;
	for (const auto& object : m_Consumables)
	{
		Objects.Add(object.Value);
	}

	return Objects;
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

void UInventoryComponent::OnObjectEquiped(EItemCategory Category, uint8 ItemIndex, bool bInMainHand)
{
	if (m_SkeletalMesh)
	{
		FInventoryItem Item;
		if (UInventoryDatabase::GetInventoryItem(Category, ItemIndex, Item))
		{
			if (Item.Class)
			{
				FActorSpawnParameters params;
				params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				FTransform ObjectTransform(FRotator::ZeroRotator, FVector::ZeroVector);
				AHeldObject* HeldObject = Cast<AHeldObject>(GetWorld()->SpawnActor(Item.Class, &ObjectTransform, params));
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

					}
					else // Attatching to left hand
					{
						m_LeftHandObject = HeldObject;
						m_LeftHandObject->AttachToComponent(m_SkeletalMesh, rules, LHandSocketName);
					}
				}
				else
				{
					UE_LOG(LogNotice, Warning, TEXT("<InventoryComponent>: Failed to spawn weapon."));
				}
			}
			else
			{
				UE_LOG(LogNotice, Warning, TEXT("<InventoryComponent>: Attempted to wield %s but no visual class was found."), *Item.Name.ToString());
			}
		}
		else
		{
			UE_LOG(LogNotice, Warning, TEXT("<InventoryComponent>: No Visual weapon with index %i exists. Object spawn failed."), ItemIndex);
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
			m_RightHandObject->Destroy();
			m_RightHandObject = nullptr;
		}

	}
	else // Detaching from left hand
	{
		if (m_LeftHandObject)
		{
			m_LeftHandObject->Destroy();
			m_LeftHandObject = nullptr;
		}
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
			result = (bool)m_Weapons[Name].Quantity;
		break;
	case EItemCategory::CONSUMABLE:
		if (m_Consumables.Contains(Name))
			result = (bool)m_Consumables[Name].Quantity;
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