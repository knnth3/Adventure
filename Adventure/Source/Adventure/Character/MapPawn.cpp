// Fill out your copyright notice in the Description page of Project Settings.

#include "MapPawn.h"
#include "Basics.h"
#include "Components/InputComponent.h"

// Sets default values
AMapPawn::AMapPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bMoving = false;
}

// Called when the game starts or when spawned
void AMapPawn::BeginPlay()
{
	Super::BeginPlay();
	
	m_destination = GetActorLocation();
}

// Called every frame
void AMapPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MovePawn(DeltaTime);
}

// Called to bind functionality to input
void AMapPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AMapPawn::SetDestination(FVector WorldLocation)
{
	m_destination.X = WorldLocation.X;
	m_destination.Y = WorldLocation.Y;
	m_destination.Z = GetActorLocation().Z;

	bMoving = true;
}

void AMapPawn::MovePawn(float DeltaTime)
{
	if (bMoving)
	{
		FVector CurrentLocation = GetActorLocation();
		FVector TravelVector = m_destination - CurrentLocation;

		float ErrorMarginInCM = 10.0f;
		if (TravelVector.Size() >= ErrorMarginInCM)
		{
			FVector DeltaLocation = TravelVector.GetSafeNormal() * (m_stats.MoveSpeed * 100) * DeltaTime;
			AddActorWorldOffset(DeltaLocation);
		}
		else
		{
			AddActorWorldOffset(TravelVector);

			bMoving = false;
		}
	}
}

