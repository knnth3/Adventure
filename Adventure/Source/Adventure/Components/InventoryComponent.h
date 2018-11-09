// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include <map>
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

USTRUCT(BlueprintType)
struct FInventoryObjectEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	uint8 ObjectEnum;

	UPROPERTY(BlueprintReadWrite)
	int Count;

};

USTRUCT(BlueprintType)
struct FCustomInventoryObjectEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FName Name;
	UPROPERTY(BlueprintReadWrite)
	int Count;

	UPROPERTY(BlueprintReadWrite)
	int Weight;

	UPROPERTY(BlueprintReadWrite)
	FString Description;

};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class ADVENTURE_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddObject(uint8 objectEnum, int quantity);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveObject(uint8 objectEnum, int quantity);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddCustomObject(FCustomInventoryObjectEntry object);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveCustomObject(FName name, int quantity);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool EquipObject(uint8 objectEnum, bool bInMainHand = true);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UnequipObject(bool inRightHand = true);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int GetObjectCount(uint8 objectEnum)const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int GetCarryCapacity() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int GetCurrentCarryWeight() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetCarryCapacity(int newCapacity);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<FInventoryObjectEntry> GetInventory() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<FCustomInventoryObjectEntry> GetCustomInventoryObjects() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool GetCustomInventoryObject(FName Name, FCustomInventoryObjectEntry& object) const;

protected:

	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory Events")
	void OnObjectEquiped(uint8 objectEnum, bool bInMainHand);

	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory Events")
	void OnObjectUnequiped(bool bInMainHand);

	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory Events")
	void OnObjectAdded(uint8 objectEnum);

	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory Events")
	void OnObjectRemoved(uint8 objectEnum);

	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory Events")
	void OnCustomObjectAdded(FName Name);

	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory Events")
	void OnCustomObjectRemoved(FName Name);

	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory Events")
	void OnBecomeOverburdened(bool newState);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory Object Events")
	void QueryObjectWeight(int& weight, uint8 objectEnum) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory Object Events")
	void QueryObjectCategory(uint8& objectClassEnum, uint8 objectEnum) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory Object Events")
	void QueryObjectInfo(int& weight, uint8& objectClassEnum, uint8 objectEnum) const;

private:

	bool ContainsObject(uint8 objectEnum) const;
	bool ContainsCustomObject(FName name) const;

	bool m_bOverburdened;
	int m_MaxWeight;
	int m_CurrentWeight;
	int m_MainHandObject;
	int m_OffHandObject;
	std::map<uint8, int> m_InventoryObjects;
	std::map<FName, FCustomInventoryObjectEntry> m_CustomInventoryObjects;

};
