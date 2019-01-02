// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "InteractionInterfaceComponent.h"
#include "DataTables/AttackDatabase.h"
#include "Components/StatisticsComponent.h"
#include "Adventure.h"
#include "Character/MapPawn.h"

// Sets default values for this component's properties
UInteractionInterfaceComponent::UInteractionInterfaceComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	m_Stats = nullptr;
	// ...
}

// Used to refrence/modify pawn stats on interaction
void UInteractionInterfaceComponent::AttachStatistics(UStatisticsComponent * Statistics)
{
	m_Stats = Statistics;
}

void UInteractionInterfaceComponent::SetActiveAttack(FAttackReuqest Request)
{
	m_ActiveAttack = Request;
	m_bNoAttackSet = false;
}

void UInteractionInterfaceComponent::Attack()
{
	auto attackClass = UAttackDatabase::GetAttack(m_ActiveAttack.AttackID);
	if (!m_bNoAttackSet && attackClass)
	{
		if (m_Stats)
		{
			FActorSpawnParameters params;
			params.Owner = GetOwner();
			params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			FVector StartLocation = GetOwner()->GetActorLocation();
			FRotator Rotation = GetOwner()->GetActorRotation();
			AAttack* Attack = GetWorld()->SpawnActor<AAttack>(*attackClass, StartLocation, Rotation, params);
			Attack->Initialize(m_ActiveAttack);
		}
		else
		{
			UE_LOG(LogNotice, Warning, TEXT("<InteractionComponent>: Statistics component was not found on parent."));
		}
	}
	else
	{
		UE_LOG(LogNotice, Warning, TEXT("<InteractionComponent>: No Attack with index of %i exists."), m_ActiveAttack.AttackID);
	}
}

void UInteractionInterfaceComponent::TakeDamage(int DamageValue, FVector AttackerLocation)
{
	if (m_Stats && m_Stats->GetCurrentAction() != PAWN_ACTION::DEAD)
	{
		m_Stats->SetCurrentAction(PAWN_ACTION::DAMAGED);
		m_Stats->SetCurrentHealth(m_Stats->GetCurrentHealth() - DamageValue);

		Cast<AMapPawn>(GetOwner())->ServerOnly_SetTargetLocation(AttackerLocation);
	}
}

bool UInteractionInterfaceComponent::Celebrate(uint8 AnimIndex)
{
	return false;
}

// Called when the game starts
void UInteractionInterfaceComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

