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

#include "Components/InteractionInterfaceComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Attack.generated.h"

UCLASS()
class ADVENTURE_API AAttack : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAttack();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Initialize(FAttackReuqest Request);

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int Damage;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Radius;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector Destination;

	UFUNCTION(BlueprintImplementableEvent)
	void OnInitialized();
	
};
