// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MapPawn.generated.h"



USTRUCT(BlueprintType)
struct ADVENTURE_API FStatSheet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Sheet")
	int Health = 10;

	//Meters per second
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Sheet")
	float MoveSpeed = 10;

};

UCLASS()
class ADVENTURE_API AMapPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMapPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "Stat Sheet")
	void SetDestination(FVector WorldLocation);

private:

	void MovePawn(float DeltaTime);

	bool bMoving;
	FStatSheet m_stats;
	FVector m_destination;
};
