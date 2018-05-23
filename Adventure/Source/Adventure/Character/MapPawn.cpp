// Fill out your copyright notice in the Description page of Project Settings.
#include "MapPawn.h"

#include "Grid/WorldGrid.h"
#include "Adventure.h"

// Sets default values
AMapPawn::AMapPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bMoving = false;
	bReplicates = true;

	CameraSettings.AngularVelocity = 240.0f;
	CameraSettings.ZoomSpeed = TO_CENTIMETERS(50.0f);
	CameraSettings.MaxUpRotation = -85.0f;
	CameraSettings.MaxDownRotation = -45.0f;
	CameraSettings.MaxOutZoom = TO_CENTIMETERS(8.0f);
	CameraSettings.MaxInZoom = TO_CENTIMETERS(2.0f);

	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	//Create a static mesh component
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Focus"));
	RootComponent = Scene;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(Scene);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = false; // Rotate the arm based on the controller
	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->CameraRotationLagSpeed = 10.0f;
												 // Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
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
void AMapPawn::SetDestination(FGridCoordinate GridLocation)
{
	/** Possible Optimization */
	//if (!HasAuthority())
	//{
	//	TActorIterator<AWorldGrid> GridItr(GetWorld());
	//	if (GridItr)
	//	{
	//		if (!GridItr->IsOccupied(GridLocation))
	//		{
	//			FVector WorldLocation = UGridFunctions::GridToWorldLocation(GridLocation);
	//			FVector DistanceBetween = WorldLocation - GetActorLocation();

	//			if (DistanceBetween.Size2D() > 0.0f)
	//			{
	//				m_destination.X = WorldLocation.X;
	//				m_destination.Y = WorldLocation.Y;
	//				m_destination.Z = GetActorLocation().Z;

	//				bMoving = true;
	//			}
	//		}
	//	}
	//}

	Server_SetDestination(GridLocation);
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
		if (HasAuthority())
		{
			TActorIterator<AWorldGrid> GridItr(GetWorld());
			if (GridItr)
			{
				FVector CurrentLocation = GetActorLocation();
				FVector TravelVector = m_destination - CurrentLocation;
				FVector DeltaLocation = TravelVector.GetSafeNormal() * (m_stats.MoveSpeed * 100) * DeltaTime;

				//Travel only the distance needed to reach the destination
				if (TravelVector.Size2D() < DeltaLocation.Size2D())
				{
					DeltaLocation = TravelVector;
					bMoving = false;
				}

				//If our next path is blocked, stop.
				FGridCoordinate pos = UGridFunctions::WorldToGridLocation(CurrentLocation);
				FGridCoordinate dest = UGridFunctions::WorldToGridLocation(CurrentLocation + DeltaLocation);
				if (!(pos == dest) &&!GridItr->MovePawn(pos, dest))
				{
					m_destination = UGridFunctions::GridToWorldLocation(pos);
					bMoving = true;
				}
				else
				{
					AddActorWorldOffset(DeltaLocation);
				}
			}
		}
		else
		{
			//Clients can do the normal work based off the info recieved from the server
			FVector CurrentLocation = GetActorLocation();
			FVector TravelVector = m_destination - CurrentLocation;
			FVector DeltaLocation = TravelVector.GetSafeNormal() * (m_stats.MoveSpeed * 100) * DeltaTime;

			//Travel only the distance needed to reach the destination
			if (TravelVector.Size2D() < DeltaLocation.Size2D())
			{
				DeltaLocation = TravelVector;
				bMoving = false;
			}

			AddActorWorldOffset(DeltaLocation);
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

void AMapPawn::Server_SetDestination_Implementation(FGridCoordinate GridLocation)
{
	TActorIterator<AWorldGrid> GridItr(GetWorld());
	if (GridItr)
	{
		if (!GridItr->IsOccupied(GridLocation))
		{
			FVector WorldLocation = UGridFunctions::GridToWorldLocation(GridLocation);
			FVector DistanceBetween = WorldLocation - GetActorLocation();

			if (DistanceBetween.Size2D() > 0.0f)
			{
				m_destination.X = WorldLocation.X;
				m_destination.Y = WorldLocation.Y;
				m_destination.Z = GetActorLocation().Z;

				bMoving = true;
			}
		}
	}
}

bool AMapPawn::Server_SetDestination_Validate(FGridCoordinate WorldLocation)
{
	return true;
}