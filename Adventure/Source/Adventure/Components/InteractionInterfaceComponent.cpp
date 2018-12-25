// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "InteractionInterfaceComponent.h"

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
void UInteractionInterfaceComponent::AttatchStatistics(UStatisticsComponent * Statistics)
{
	m_Stats = Statistics;
}

bool UInteractionInterfaceComponent::Attack(uint8 AnimIndex, FVector Location)
{
	return false;
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

