// Fill out your copyright notice in the Description page of Project Settings.

#include "PickUp.h"

APickUp::APickUp()
{
	PrimaryActorTick.bCanEverTick = true;
	SetMobility(EComponentMobility::Movable);
}

void APickUp::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SetReplicates(true);
		SetReplicateMovement(true);

		m_originalPosition = GetActorLocation();
	}
}

void APickUp::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority())
	{
		FVector up = FVector(0, 0, 1);
		FVector location = m_originalPosition;

		float height = Magnitude * sinf((2/Length) * PI * m_elapsedTime);

		location += (up * height);
		SetActorLocation(location);
		m_elapsedTime += DeltaSeconds;

		DeltaSeconds = fmodf(DeltaSeconds, Length);
	}
}
