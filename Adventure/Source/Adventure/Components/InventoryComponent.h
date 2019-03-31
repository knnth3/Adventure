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

#include <map>
#include "DataTables/InventoryDatabase.h"
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponDelegate, bool, RightHand);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class ADVENTURE_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

	void AttachStatistics(class UStatisticsComponent* Statistics);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool GiveConsumable(FName Name, int Quantity);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool GiveWeapon(FName Name, int Quantity);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void GiveCurrency(int Gold, int Silver, int Copper);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveConsumable(const FName& Name, int Quantity);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveWeapon(const FName& Name, int Quantity);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RemoveCurrency(int Gold, int Silver, int Copper);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool EquipWeapon(FName Name, bool bInMainHand = true);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UnequipWeapon(bool inRightHand = true);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int GetObjectCount(uint8 objectEnum)const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int GetCarryCapacity() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int GetCurrentCarryWeight() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetCarryCapacity(int newCapacity);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<FName> GetWeapons() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<FName> GetConsumables() const;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Startup Settings")
	FName RHandSocketName;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Startup Settings")
	FName LHandSocketName;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Startup Settings")
	FName SkeletalMeshTag;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Startup Settings")
	bool bIsLeftHanded;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	class AHeldObject* GetWeapon(bool rightHand)const;

	UPROPERTY(BlueprintAssignable, Category = "Visual Changes")
	FWeaponDelegate OnWeaponEquiped;

	UPROPERTY(BlueprintAssignable, Category = "Visual Changes")
	FWeaponDelegate OnWeaponUnequiped;

protected:

	// Called when the game starts
	virtual void BeginPlay() override;
	// Called when component is destroyed
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	void OnObjectEquiped(EItemCategory Category, const FName& Name, bool bInMainHand);
	void OnObjectUnequiped(bool bInMainHand);
	void OnOverburdenedStateChanged();
	bool ContainsObject(EItemCategory Category, const FName Name) const;
	bool AddWeight(int deltaWeight);
	FItemVisualsRow* GetItemVisual(EItemCategory Category, const FName& Name);

	int m_Gold;
	int m_Silver;
	int m_Copper;
	bool m_bOverburdened;
	uint32 m_MaxWeight;
	uint32 m_CurrentWeight;
	FName m_MainHandObject;
	FName m_OffHandObject;
	class AHeldObject* m_RightHandObject;
	class AHeldObject* m_LeftHandObject;
	class UStatisticsComponent* m_Stats;
	class USkeletalMeshComponent* m_SkeletalMesh;
	TMap<FName, uint32> m_Consumables;
	TMap<FName, uint32> m_Weapons;

};
