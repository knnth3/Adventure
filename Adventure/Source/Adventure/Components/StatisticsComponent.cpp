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

#include "StatisticsComponent.h"

// Sets default values for this component's properties
UStatisticsComponent::UStatisticsComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	m_MoveSpeed = 30.f;
	m_CurrentHealth = 10;
	m_CurrentLevel = 1;
	m_StatusEffect = STATUS_EFFECT::NONE;
	m_WeaponStance = WEAPON_TYPE::UNARMED;
	m_StatUpdateVersion = 0;
}

int UStatisticsComponent::GetCurrentInformationVersion()
{
	return m_StatUpdateVersion;
}

FName UStatisticsComponent::GetCharacterName() const
{
	return m_Name;
}

FName UStatisticsComponent::GetCharacterClass() const
{
	return m_BaseStats.Class;
}

float UStatisticsComponent::GetMoveSpeed() const
{
	return m_MoveSpeed;
}

int UStatisticsComponent::GetMaxHealth() const
{
	return m_BaseStats.MaxHealth;
}

int UStatisticsComponent::GetCurrentHealth() const
{
	return m_CurrentHealth;
}

int UStatisticsComponent::GetCurrentAC() const
{
	return m_BaseStats.AC;
}

int UStatisticsComponent::GetCurrentLevel() const
{
	return m_CurrentLevel;
}

int UStatisticsComponent::GetStrength() const
{
	return m_BaseStats.Strength;
}

int UStatisticsComponent::GetDexterity() const
{
	return m_BaseStats.Dexterity;
}

int UStatisticsComponent::GetConstitution() const
{
	return m_BaseStats.Constitution;
}

int UStatisticsComponent::GetIntelligence() const
{
	return m_BaseStats.Intelligence;
}

int UStatisticsComponent::GetWisdom() const
{
	return m_BaseStats.Wisdom;
}

int UStatisticsComponent::GetCharisma() const
{
	return m_BaseStats.Charisma;
}

STATUS_EFFECT UStatisticsComponent::GetCurrentStatusEffect() const
{
	return m_StatusEffect;
}

WEAPON_TYPE UStatisticsComponent::GetCurrentWeaponStance() const
{
	return m_WeaponStance;
}

PAWN_ACTION UStatisticsComponent::GetCurrentAction() const
{
	return m_CurrentAction;
}

void UStatisticsComponent::SetCharacterName(FName Name)
{
	m_Name = Name;
	m_StatUpdateVersion++;
}

void UStatisticsComponent::SetCharacterClass(FName Class)
{
	m_BaseStats.Class = Class;
	m_StatUpdateVersion++;
}

void UStatisticsComponent::SetMoveSpeed(float Speed)
{
	m_MoveSpeed = Speed;
	m_StatUpdateVersion++;
}

void UStatisticsComponent::SetMaxHealth(int Health)
{
	m_BaseStats.MaxHealth = Health;
	m_StatUpdateVersion++;
}

void UStatisticsComponent::SetCurrentHealth(int Health)
{
	m_CurrentHealth = Health;
	m_StatUpdateVersion++;
	OnHealthChanged.Broadcast(Health);
	if (m_CurrentHealth <= 0)
	{
		m_CurrentAction = PAWN_ACTION::DEAD;
	}
}

void UStatisticsComponent::SetCurrentAC(int AC)
{
	m_BaseStats.AC = AC;
	m_StatUpdateVersion++;
	OnACChanged.Broadcast(AC);
}

void UStatisticsComponent::SetCurrentLevel(uint8 Level)
{
	m_CurrentLevel = Level;
	m_StatUpdateVersion++;
}

void UStatisticsComponent::SetStrength(uint8 Strength)
{
	m_BaseStats.Strength = Strength;
	m_StatUpdateVersion++;
}

void UStatisticsComponent::SetDexterity(uint8 Dexterity)
{
	m_BaseStats.Dexterity = Dexterity;
	m_StatUpdateVersion++;
}

void UStatisticsComponent::SetConstitution(uint8 Constitution)
{
	m_BaseStats.Constitution = Constitution;
	m_StatUpdateVersion++;
}

void UStatisticsComponent::SetIntelligence(uint8 Intelligence)
{
	m_BaseStats.Intelligence = Intelligence;
	m_StatUpdateVersion++;
}

void UStatisticsComponent::SetWisdom(uint8 Wisdom)
{
	m_BaseStats.Wisdom = Wisdom;
	m_StatUpdateVersion++;
}

void UStatisticsComponent::SetCharisma(uint8 Charisma)
{
	m_BaseStats.Charisma = Charisma;
	m_StatUpdateVersion++;
}

void UStatisticsComponent::SetStatusEffect(STATUS_EFFECT Effect)
{
	m_StatusEffect = Effect;
	m_StatUpdateVersion++;
}

void UStatisticsComponent::SetWeaponStance(WEAPON_TYPE Stance)
{
	m_WeaponStance = Stance;
	m_StatUpdateVersion++;
}

void UStatisticsComponent::SetCurrentAction(PAWN_ACTION Action)
{
	m_CurrentAction = Action;
}

// Called when the game starts
void UStatisticsComponent::BeginPlay()
{
	Super::BeginPlay();
	
}