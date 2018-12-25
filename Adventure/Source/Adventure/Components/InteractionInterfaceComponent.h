// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractionInterfaceComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ADVENTURE_API UInteractionInterfaceComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInteractionInterfaceComponent();

	// Used to refrence/modify pawn stats on interaction
	void AttatchStatistics(class UStatisticsComponent* Statistics);

	UFUNCTION(BlueprintCallable, Category = "Interactions")
	bool Attack(uint8 AnimIndex, FVector Location);

	UFUNCTION(BlueprintCallable, Category = "Interactions")
	bool Celebrate(uint8 AnimIndex);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:

	class UStatisticsComponent* m_Stats;
		
};
