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
		if(component)
			component->SetRenderCustomDepth(true);
	}
}

void UHighlightComponent::DimObjects()
{
	for (auto& component : m_HighlightableComponents)
	{
		if(component)
			component->SetRenderCustomDepth(false);
	}
}

