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

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HighlightComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ADVENTURE_API UHighlightComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHighlightComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Highlight Component")
	void AddHighlightableComponents(TArray<UPrimitiveComponent*> Components);

	UFUNCTION(BlueprintCallable, Category = "Highlight Component")
	void HighlightObjects();

	UFUNCTION(BlueprintCallable, Category = "Highlight Component")
	void DimObjects();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:	

	TArray<UPrimitiveComponent*> m_HighlightableComponents;
		
	
};
