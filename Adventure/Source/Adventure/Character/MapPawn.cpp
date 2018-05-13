// Fill out your copyright notice in the Description page of Project Settings.

#include "MapPawn.h"
#include "Basics.h"
#include "DrawDebugHelpers.h"
#include "Adventure.h"
#include "Components/InputComponent.h"

FString GetStringOf(ENetRole Role)
{
	switch (Role)
	{
	case ROLE_None:
		return "Role: None";
	case ROLE_SimulatedProxy:
		return "Role: Simulated Proxy";
	case ROLE_AutonomousProxy:
		return "Role: Autonomous Proxy";
	case ROLE_Authority:
		return "Role: Authority";
	default:
		return "Role: Error Encountered";
	}
}

// Sets default values
AMapPawn::AMapPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bMoving = false;
	bReplicates = true;
}

// Called when the game starts or when spawned
void AMapPawn::BeginPlay()
{
	Super::BeginPlay();
	
	m_destination = GetActorLocation();

	if (HasAuthority())
	{
		m_stats.Health = (FMath::Rand() % 100) + 1;
	}
}

// Called every frame
void AMapPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MovePawn(DeltaTime);

	FString HealthValue = "Health: " + FString::FromInt(m_stats.Health);
	FString IsMoving = "Moving: " + FString::FromInt(bMoving);

	GetStringOf(Role);
	DrawDebugString(GetWorld(), FVector(0,0,100), IsMoving, this, FColor::White, DeltaTime);
}

// Called to bind functionality to input
void AMapPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

//Set the pawns destination location
void AMapPawn::SetDestination(FVector WorldLocation)
{
	m_destination.X = WorldLocation.X;
	m_destination.Y = WorldLocation.Y;
	m_destination.Z = GetActorLocation().Z;

	bMoving = true;

	Server_SetDestination(WorldLocation);
}

//Returns the pawns stats
FStatSheet AMapPawn::GetStatSheet() const
{
	return m_stats;
}

//Moves a pawn if its destination is not the same as its position
void AMapPawn::MovePawn(float DeltaTime)
{
	if (bMoving)
	{
		FVector CurrentLocation = GetActorLocation();
		FVector TravelVector = m_destination - CurrentLocation;
		FVector DeltaLocation = TravelVector.GetSafeNormal() * (m_stats.MoveSpeed * 100) * DeltaTime;

		if (TravelVector.Size() >= DeltaLocation.Size())
		{
			AddActorWorldOffset(DeltaLocation);
		}
		else
		{
			AddActorWorldOffset(TravelVector);

			bMoving = false;
		}
	}
}

void AMapPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMapPawn, m_stats);
	DOREPLIFETIME(AMapPawn, bMoving);
	DOREPLIFETIME(AMapPawn, m_destination);
}

//Server functions

void AMapPawn::Server_SetDestination_Implementation(FVector WorldLocation)
{
	m_destination.X = WorldLocation.X;
	m_destination.Y = WorldLocation.Y;
	m_destination.Z = GetActorLocation().Z;

	bMoving = true;
}

bool AMapPawn::Server_SetDestination_Validate(FVector WorldLocation)
{
	return true;
}