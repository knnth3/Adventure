// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "HighlightComponent.h"
#include "Components/PrimitiveComponent.h"


// Sets default values for this component's properties
UHighlightComponent::UHighlightComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UHighlightComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UHighlightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UHighlightComponent::AddHighlightableComponents(TArray<UPrimitiveComponent*> Components)
{
	m_HighlightableComponents.Append(Components);
}

void UHighlightComponent::HighlightObjects()
{
	for (auto& component : m_HighlightableComponents)
	{
		component->SetRenderCustomDepth(true);
	}
}

void UHighlightComponent::DimObjects()
{
	for (auto& component : m_HighlightableComponents)
	{
		component->SetRenderCustomDepth(false);
	}
}

