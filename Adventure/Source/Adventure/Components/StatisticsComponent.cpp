// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "StatisticsComponent.h"

// Sets default values for this component's properties
UStatisticsComponent::UStatisticsComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	m_MoveSpeed = 30.f;
	m_MaxHealth = 10;
	m_CurrentHealth = 10;
	m_CurrentLevel = 1;
	m_Strength = 0;
	m_Dexterity = 0;
	m_Constitution = 0;
	m_Intelligence = 0;
	m_Wisdom = 0;
	m_Charisma = 0;
	m_StatusEffect = STATUS_EFFECT::NONE;
	m_WeaponStance = WEAPON_TYPE::UNARMED;
}

FName UStatisticsComponent::GetCharacterName() const
{
	return m_Name;
}

FName UStatisticsComponent::GetCharacterClass() const
{
	return m_Class;
}

float UStatisticsComponent::GetMoveSpeed() const
{
	return m_MoveSpeed;
}

int UStatisticsComponent::GetMaxHealth() const
{
	return m_MaxHealth;
}

int UStatisticsComponent::GetCurrentHealth() const
{
	return m_CurrentHealth;
}

int UStatisticsComponent::GetCurrentLevel() const
{
	return m_CurrentLevel;
}

int UStatisticsComponent::GetStrength() const
{
	return m_Strength;
}

int UStatisticsComponent::GetDexterity() const
{
	return m_Dexterity;
}

int UStatisticsComponent::GetConstitution() const
{
	return m_Constitution;
}

int UStatisticsComponent::GetIntelligence() const
{
	return m_Intelligence;
}

int UStatisticsComponent::GetWisdom() const
{
	return m_Wisdom;
}

int UStatisticsComponent::GetCharisma() const
{
	return m_Charisma;
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
}

void UStatisticsComponent::SetCharacterClass(FName Class)
{
	m_Class = Class;
}

void UStatisticsComponent::SetMoveSpeed(float Speed)
{
	m_MoveSpeed = Speed;
}

void UStatisticsComponent::SetMaxHealth(int Health)
{
	m_MaxHealth = Health;
}

void UStatisticsComponent::SetCurrentHealth(int Health)
{
	m_CurrentHealth = Health;
}

void UStatisticsComponent::SetCurrentLevel(uint8 Level)
{
	m_CurrentLevel = Level;
}

void UStatisticsComponent::SetStrength(uint8 Strength)
{
	m_Strength = Strength;
}

void UStatisticsComponent::SetDexterity(uint8 Dexterity)
{
	m_Dexterity = Dexterity;
}

void UStatisticsComponent::SetConstitution(uint8 Constitution)
{
	m_Constitution = Constitution;
}

void UStatisticsComponent::SetIntelligence(uint8 Intelligence)
{
	m_Intelligence = Intelligence;
}

void UStatisticsComponent::SetWisdom(uint8 Wisdom)
{
	m_Wisdom = Wisdom;
}

void UStatisticsComponent::SetCharisma(uint8 Charisma)
{
	m_Charisma = Charisma;
}

void UStatisticsComponent::SetStatusEffect(STATUS_EFFECT Effect)
{
	m_StatusEffect = Effect;
}

void UStatisticsComponent::SetWeaponStance(WEAPON_TYPE Stance)
{
	m_WeaponStance = Stance;
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