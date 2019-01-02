// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

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
