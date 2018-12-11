// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "PickUp.generated.h"

/**
 * 
 */
UCLASS()
class ADVENTURE_API APickUp : public AStaticMeshActor
{
	GENERATED_BODY()

	APickUp();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;


	UPROPERTY(EditAnywhere)
	float Speed;

	UPROPERTY(EditAnywhere)
	float Magnitude;

	//In Seconds
	UPROPERTY(EditAnywhere)
	float Length;

private:
	UPROPERTY(Replicated)
	float m_elapsedTime;

	UPROPERTY(Replicated)
	FVector m_originalPosition;
	
};
