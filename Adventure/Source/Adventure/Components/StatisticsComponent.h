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

#include "DataTables/PawnDatabase.h"
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatisticsComponent.generated.h"


//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFloatValueChangedDelegate, float);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FIntDelegate, int, Value);

UENUM(BlueprintType)
enum class STATUS_EFFECT : uint8
{
	NONE,
	FROZEN,
	BURNED,
	PARALYZED,
	BLIND,
	SILENCED,
	FATIGUED,
	BLEEDING,
	SLOWED,
};

UENUM(BlueprintType)
enum class PAWN_ACTION : uint8
{
	NONE,
	ATTACKING,
	SLEEPING,
	RESTING,
	CELEBRATING,
	MOVING,
	DEAD,
	DAMAGED
};

UENUM(BlueprintType)
enum class WEAPON_TYPE : uint8
{
	UNARMED,
	HAMMER_1H,
	HAMMER_2H,
	STAFF_1H,
	STAFF_2H,
	SWORD_1H,
	SWORD_2H,
	CROSSBOW_1H,
	CROSSBOW_2H
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class ADVENTURE_API UStatisticsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStatisticsComponent();

	// Getters ----------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "Information Version Control")
	int GetCurrentInformationVersion();

	UFUNCTION(BlueprintCallable, Category = "Basic Information")
	FName GetCharacterName()const;

	UFUNCTION(BlueprintCallable, Category = "Basic Information")
	FName GetCharacterClass()const;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	float GetMoveSpeed()const;

	UFUNCTION(BlueprintCallable, Category = "Basic Information")
	int GetMaxHealth()const;

	UFUNCTION(BlueprintCallable, Category = "Basic Information")
	int GetCurrentHealth()const;

	UFUNCTION(BlueprintCallable, Category = "Basic Information")
	int GetCurrentAC()const;

	UFUNCTION(BlueprintCallable, Category = "Basic Information")
	int GetCurrentLevel()const;

	UFUNCTION(BlueprintCallable, Category = "Base Stats")
	int GetStrength()const;

	UFUNCTION(BlueprintCallable, Category = "Base Stats")
	int GetDexterity()const;

	UFUNCTION(BlueprintCallable, Category = "Base Stats")
	int GetConstitution()const;

	UFUNCTION(BlueprintCallable, Category = "Base Stats")
	int GetIntelligence()const;

	UFUNCTION(BlueprintCallable, Category = "Base Stats")
	int GetWisdom()const;

	UFUNCTION(BlueprintCallable, Category = "Base Stats")
	int GetCharisma()const;

	UFUNCTION(BlueprintCallable, Category = "Status")
	STATUS_EFFECT GetCurrentStatusEffect()const;

	UFUNCTION(BlueprintCallable, Category = "Status")
	WEAPON_TYPE GetCurrentWeaponStance()const;

	UFUNCTION(BlueprintCallable, Category = "Status")
	PAWN_ACTION GetCurrentAction()const;

	// Setters ----------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "Basic Information")
	void SetCharacterName(FName Name);

	UFUNCTION(BlueprintCallable, Category = "Basic Information")
	void SetCharacterClass(FName Class);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetMoveSpeed(float Speed);

	UFUNCTION(BlueprintCallable, Category = "Basic Information")
	void SetMaxHealth(int Health);

	UFUNCTION(BlueprintCallable, Category = "Basic Information")
	void SetCurrentHealth(int Health);

	UFUNCTION(BlueprintCallable, Category = "Basic Information")
	void SetCurrentAC(int AC);

	UFUNCTION(BlueprintCallable, Category = "Basic Information")
	void SetCurrentLevel(uint8 Level);

	UFUNCTION(BlueprintCallable, Category = "Base Stats")
	void SetStrength(uint8 Strength);

	UFUNCTION(BlueprintCallable, Category = "Base Stats")
	void SetDexterity(uint8 Dexterity);

	UFUNCTION(BlueprintCallable, Category = "Base Stats")
	void SetConstitution(uint8 Constitution);

	UFUNCTION(BlueprintCallable, Category = "Base Stats")
	void SetIntelligence(uint8 Intelligence);

	UFUNCTION(BlueprintCallable, Category = "Base Stats")
	void SetWisdom(uint8 Wisdom);

	UFUNCTION(BlueprintCallable, Category = "Base Stats")
	void SetCharisma(uint8 Charisma);

	UFUNCTION(BlueprintCallable, Category = "Status")
	void SetStatusEffect(STATUS_EFFECT Effect);

	UFUNCTION(BlueprintCallable, Category = "Status")
	void SetWeaponStance(WEAPON_TYPE Stance);

	UFUNCTION(BlueprintCallable, Category = "Status")
	void SetCurrentAction(PAWN_ACTION Action);

	// Delegates ------------------------------------------------
	UPROPERTY(BlueprintAssignable, Category = "Statistic Changes")
	FIntDelegate OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Statistic Changes")
	FIntDelegate OnACChanged;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:

	int m_StatUpdateVersion;

	// Pawn Name
	FName m_Name;

	// Base stats
	FPawnDefaultStats m_BaseStats;

	//Units in feet (walking)
	float m_MoveSpeed;

	// Current health pool
	int16 m_CurrentHealth;

	// Current pawn level
	uint8 m_CurrentLevel;

	// CurrentStatusEffect
	STATUS_EFFECT m_StatusEffect;

	// Current equiped weapon stance
	WEAPON_TYPE m_WeaponStance;

	// Current action taken by pawn
	PAWN_ACTION m_CurrentAction;
		
};
