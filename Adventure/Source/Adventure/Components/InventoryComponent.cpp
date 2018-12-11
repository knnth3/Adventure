// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "InventoryComponent.h"


// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	m_bOverburdened = false;
	m_MainHandObject = -1;
	m_OffHandObject = -1;
	// ...
}

bool UInventoryComponent::ContainsObject(uint8 objectEnum) const
{
	return (m_InventoryObjects.find(objectEnum) != m_InventoryObjects.end());
}

bool UInventoryComponent::ContainsCustomObject(FName name) const
{
	return (m_CustomInventoryObjects.find(name) != m_CustomInventoryObjects.end());
}

bool UInventoryComponent::AddObject(uint8 objectEnum, int quantity)
{
	int weight = 0;
	QueryObjectWeight(weight, objectEnum);

	int canAddNumber = quantity;

	if (weight > 0)
	{
		canAddNumber = (m_MaxWeight - m_CurrentWeight) / weight;
		canAddNumber = (canAddNumber > quantity) ? quantity : canAddNumber;
	}

	if (canAddNumber > 0)
	{
		m_CurrentWeight += weight * canAddNumber;

		if (!ContainsObject(objectEnum))
		{
			m_InventoryObjects[objectEnum] = canAddNumber;
		}
		else
		{
			m_InventoryObjects[objectEnum] += canAddNumber;
		}

		OnObjectAdded(objectEnum);

		return true;
	}

	return false;
}

bool UInventoryComponent::RemoveObject(uint8 objectEnum, int quantity)
{
	if (ContainsObject(objectEnum))
	{
		int canRemoveNumber = (m_InventoryObjects[objectEnum] > quantity) ? quantity : m_InventoryObjects[objectEnum];

		int weight = 0;
		QueryObjectWeight(weight, objectEnum);

		m_CurrentWeight -= weight * canRemoveNumber;

		m_InventoryObjects[objectEnum] -= canRemoveNumber;

		OnObjectRemoved(objectEnum);
		return true;
	}

	return false;
}

bool UInventoryComponent::AddCustomObject(FCustomInventoryObjectEntry object)
{
	int canAddNumber = object.Count;

	if (object.Weight > 0)
	{
		canAddNumber = (m_MaxWeight - m_CurrentWeight) / object.Weight;
		canAddNumber = (canAddNumber > object.Count) ? object.Count : canAddNumber;
	}

	if (canAddNumber > 0)
	{
		m_CurrentWeight += object.Weight * canAddNumber;

		if (!ContainsCustomObject(object.Name))
		{
			m_CustomInventoryObjects[object.Name] = object;
			m_CustomInventoryObjects[object.Name].Count = canAddNumber;
		}
		else
		{
			m_CustomInventoryObjects[object.Name].Count += canAddNumber;
		}

		OnCustomObjectAdded(object.Name);

		return true;
	}

	return false;
}

bool UInventoryComponent::RemoveCustomObject(FName name, int quantity)
{
	if (ContainsCustomObject(name))
	{
		int canRemoveNumber = (m_CustomInventoryObjects[name].Count > quantity) ? quantity : m_CustomInventoryObjects[name].Count;

		m_CurrentWeight -= m_CustomInventoryObjects[name].Weight * canRemoveNumber;

		m_CustomInventoryObjects[name].Count -= canRemoveNumber;

		OnCustomObjectRemoved(name);
		return true;
	}

	return false;
}

bool UInventoryComponent::EquipObject(uint8 objectEnum, bool bInMainHand)
{
	if (ContainsObject(objectEnum))
	{
		if (m_InventoryObjects[objectEnum] > 0)
		{
			m_InventoryObjects[objectEnum]--;

			if (bInMainHand)
			{
				if (m_MainHandObject != -1)
				{
					m_InventoryObjects[m_MainHandObject]++;
				}

				m_MainHandObject = objectEnum;
			}
			else
			{
				if (m_OffHandObject != -1)
				{
					m_InventoryObjects[m_OffHandObject]++;
				}

				m_OffHandObject = objectEnum;
			}

			OnObjectEquiped(objectEnum, bInMainHand);

			return true;
		}
		
	}

	return false;
}

void UInventoryComponent::UnequipObject(bool bInMainHand)
{
	if (bInMainHand)
	{
		if (m_MainHandObject != -1)
		{
			m_InventoryObjects[m_MainHandObject]++;
		}

		m_MainHandObject = -1;
	}
	else
	{
		if (m_OffHandObject != -1)
		{
			m_InventoryObjects[m_OffHandObject]++;
		}

		m_OffHandObject = -1;
	}

	OnObjectUnequiped(bInMainHand);
}

int UInventoryComponent::GetObjectCount(uint8 objectEnum) const
{
	return 0;
}

int UInventoryComponent::GetCarryCapacity() const
{
	return m_MaxWeight;
}

int UInventoryComponent::GetCurrentCarryWeight() const
{
	return m_CurrentWeight;
}

void UInventoryComponent::SetCarryCapacity(int newCapacity)
{

	bool nowOverburdened = (m_CurrentWeight > newCapacity);
	if (m_bOverburdened != nowOverburdened)
	{
		m_bOverburdened = nowOverburdened;
		m_MaxWeight = newCapacity;
		OnBecomeOverburdened(m_bOverburdened);

	}
}

TArray<FInventoryObjectEntry> UInventoryComponent::GetInventory() const
{
	TArray<FInventoryObjectEntry> Objects;
	for (const auto& object : m_InventoryObjects)
	{
		FInventoryObjectEntry newObject;
		newObject.ObjectEnum = object.first;
		newObject.Count = object.second;

		Objects.Add(newObject);
	}
	return Objects;
}

TArray<FCustomInventoryObjectEntry> UInventoryComponent::GetCustomInventoryObjects() const
{
	TArray<FCustomInventoryObjectEntry> Objects;
	for (const auto& object : m_CustomInventoryObjects)
	{
		Objects.Add(object.second);
	}
	return Objects;
}

bool UInventoryComponent::GetCustomInventoryObject(FName Name, FCustomInventoryObjectEntry& object) const
{
	if (m_CustomInventoryObjects.find(Name) != m_CustomInventoryObjects.end())
	{
		object = m_CustomInventoryObjects.at(Name);
		return true;
	}

	return false;
}

void UInventoryComponent::QueryObjectWeight_Implementation(int & weight, uint8 objectEnum) const
{
}

void UInventoryComponent::QueryObjectCategory_Implementation(uint8 & objectClassEnum, uint8 objectEnum) const
{
}

void UInventoryComponent::QueryObjectInfo_Implementation(int & weight, uint8 & objectClassEnum, uint8 objectEnum) const
{
}
