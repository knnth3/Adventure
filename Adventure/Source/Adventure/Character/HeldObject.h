// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include "Components/StatisticsComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HeldObject.generated.h"

UCLASS()
class ADVENTURE_API AHeldObject : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHeldObject();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Game")
	WEAPON_TYPE GetWeaponStance()const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
};
