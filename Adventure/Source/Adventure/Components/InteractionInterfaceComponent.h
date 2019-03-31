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
#include "InteractionInterfaceComponent.generated.h"

USTRUCT(BlueprintType)
struct FAttackReuqest
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite)
	int AttackID = 0;

	UPROPERTY(BlueprintReadWrite)
	int Damage = 0;

	UPROPERTY(BlueprintReadWrite)
	float Radius = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	FVector Location;

};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ADVENTURE_API UInteractionInterfaceComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInteractionInterfaceComponent();

	// Used to refrence/modify pawn stats on interaction
	void AttachStatistics(class UStatisticsComponent* Statistics);

	UFUNCTION(BlueprintCallable, Category = "Interactions")
	void SetActiveAttack(FAttackReuqest Request);

	UFUNCTION(BlueprintCallable, Category = "Interactions")
	void Attack();

	UFUNCTION(BlueprintCallable, Category = "Interactions")
	void TakeDamage(int DamageValue, FVector AttackerLocation);

	UFUNCTION(BlueprintCallable, Category = "Interactions")
	bool Celebrate(uint8 AnimIndex);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:

	bool m_bNoAttackSet = true;
	FAttackReuqest m_ActiveAttack;
	class UStatisticsComponent* m_Stats;
		
};
