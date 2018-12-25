// By: Eric Marquez. All information and code provided is free to use and can be used comercially. Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include <map>
#include "DataTables/InventoryDatabase.h"
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

USTRUCT(BlueprintType)
struct FConsumable

{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FName Name;

	UPROPERTY(BlueprintReadWrite)
	int Weight;

	UPROPERTY(BlueprintReadWrite)
	FString Description;

	UPROPERTY(BlueprintReadWrite)
	int Quantity;

	UPROPERTY(BlueprintReadWrite)
	int HealthBonus;

	UPROPERTY(BlueprintReadWrite)
	int VisualIndex;

};

USTRUCT(BlueprintType)
struct FWeapon
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FName Name;

	UPROPERTY(BlueprintReadWrite)
	int Weight;

	UPROPERTY(BlueprintReadWrite)
	FString Description;

	UPROPERTY(BlueprintReadWrite)
	int Quantity;

	UPROPERTY(BlueprintReadWrite)
	int VisualIndex;

};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class ADVENTURE_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

	void AttatchStatistics(class UStatisticsComponent* Statistics);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddConsumable(const FConsumable& ConsumableInfo);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddWeapon(const FWeapon& WeaponInfo);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddCurrency(int Gold, int Silver, int Copper);

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
	TArray<FWeapon> GetWeapons() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<FConsumable> GetConsumables() const;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Startup Settings")
	FName RHandSocketName;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Startup Settings")
	FName LHandSocketName;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Startup Settings")
	FName SkeletalMeshTag;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Startup Settings")
	bool bIsLeftHanded;

protected:

	// Called when the game starts
	virtual void BeginPlay() override;

private:

	void OnObjectEquiped(EItemCategory Category, uint8 ItemID, bool bInMainHand);

	void OnObjectUnequiped(bool bInMainHand);

	void OnOverburdenedStateChanged();

	bool ContainsObject(EItemCategory Category, const FName Name) const;
	bool AddWeight(int deltaWeight);

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
	TMap<FName, FConsumable> m_Consumables;
	TMap<FName, FWeapon> m_Weapons;

};
