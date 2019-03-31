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

