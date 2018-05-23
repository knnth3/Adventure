// Fill out your copyright notice in the Description page of Project Settings.
#include "PickUp.h"

#include "Basics.h"
#include "Adventure.h"

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

	FVector up = FVector(0, 0, 1);
	FVector location = m_originalPosition;

	float height = Magnitude * sinf((2 / Length) * PI * m_elapsedTime);

	location += (up * height);
	SetActorLocation(location);

	SetActorScale3D(FVector(1.0f, 1.0f, location.Z / 4.0f));

	if (HasAuthority())
	{
		m_elapsedTime += DeltaSeconds;
		DeltaSeconds = fmodf(DeltaSeconds, Length);
	}
}

void APickUp::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APickUp, m_elapsedTime);
	DOREPLIFETIME(APickUp, m_originalPosition);
}