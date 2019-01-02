// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

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
