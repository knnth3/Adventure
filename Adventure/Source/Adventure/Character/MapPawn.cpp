// Fill out your copyright notice in the Description page of Project Settings.
#include "MapPawn.h"

#include "Grid/WorldGrid.h"
#include "Adventure.h"

// Sets default values
AMapPawn::AMapPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bMoveCharacter = false;
	bReplicates = true;
	bRotateCharacter = false;

	CameraSettings.AngularVelocity = 240.0f;
	CameraSettings.ZoomSpeed = Conversions::Meters::ToCentimeters(50.0f);
	CameraSettings.MaxUpRotation = -85.0f;
	CameraSettings.MaxDownRotation = -25.0f;
	CameraSettings.MaxOutZoom = Conversions::Meters::ToCentimeters(8.0f);
	CameraSettings.MaxInZoom = Conversions::Meters::ToCentimeters(2.0f);

	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	//Create a static mesh component
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Focus"));
	RootComponent = Scene;

	//Create a component for the pawns body
	PawnBody = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Pawn_Body"));
	PawnBody->SetupAttachment(Scene);

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
	
	FinalDestination = GetActorLocation();

	if(PawnBody)
		PawnBody->SetWorldScale3D(FVector(0.5f));

	if (HasAuthority())
	{
		CharacterStats.Health = (FMath::Rand() % 100) + 1;
	}
}

void AMapPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	TActorIterator<AWorldGrid> GridItr(GetWorld());
	if (GridItr)
	{
		GridItr->RemoveActorFromPlay(GetActorGridLocation());
	}
}

// Called every frame
void AMapPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MovePawn(DeltaTime);
	RotatePawn(DeltaTime);

	FString HealthValue = "Health: " + FString::FromInt(CharacterStats.Health);
	FString IsMoving = "Moving: " + FString::FromInt(bMoveCharacter);

	GetStringOf(Role);
	//DrawDebugString(GetWorld(), FVector(0,0,100), IsMoving, this, FColor::White, DeltaTime);
}

// Called to bind functionality to input
void AMapPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

//Set the pawns destination location
void AMapPawn::SetDestination(FGridCoordinate GridLocation)
{
	Server_SetDestination(GridLocation);
}

//Returns the pawns stats
FStatSheet AMapPawn::GetStatSheet() const
{
	return CharacterStats;
}

//Return true if the pawn is moving
bool AMapPawn::IsMoving() const
{
	return bMoveCharacter;
}

//Moves a pawn if its destination is not the same as its position
void AMapPawn::MovePawn(float DeltaTime)
{
	if (bMoveCharacter)
	{
		if (HasAuthority())
		{
			FVector DeltaLocation = GetNextMove(DeltaTime, bMoveCharacter);
			if (!bMoveCharacter)
			{
				EndMove();
			}

			//On the server, check if the delta location is valid
			//Sometimes a character might have moved to the desired location beore we have reached it
			//If so, move the pawn one space back
			if (IsMoveValid(DeltaLocation))
			{
				AddActorWorldOffset(DeltaLocation);
			}
			else
			{
				SetDestination(UGridFunctions::GridToWorldLocation(GetActorGridLocation()));
			}
		}
		else
		{
			//Clients can do the normal work based off the info recieved from the server
			FVector DeltaLocation = GetNextMove(DeltaTime, bMoveCharacter);
			if (!bMoveCharacter)
			{
				EndMove();
			}

			AddActorWorldOffset(DeltaLocation);
		}
	}
}

//Rotates a pawn if the bRotateCharacter was set (usually when setDestination was called)
void AMapPawn::RotatePawn(float DeltaTime)
{
	if (PawnBody && bRotateCharacter)
	{
		float Multiplier = CharacterStats.TurnSpeed;
		FQuat CurrentRotation(PawnBody->RelativeRotation);
		FQuat FinalRotation(Rotation);

		FQuat NextRotation = FQuat::Slerp(CurrentRotation, FinalRotation, DeltaTime * Multiplier);
		if (FinalRotation.AngularDistance(CurrentRotation) > NextRotation.AngularDistance(CurrentRotation))
		{
			PawnBody->SetWorldRotation(NextRotation);
		}
		else
		{
			PawnBody->SetWorldRotation(FinalRotation);
			bRotateCharacter = false;
		}
	}
}

FVector AMapPawn::GetNextMove(float DeltaTime, bool& bHasNextMove)
{
	FVector CurrentLocation = GetActorLocation();
	FVector TravelVector = FinalDestination - CurrentLocation;
	FVector DeltaLocation = TravelVector.GetSafeNormal() * (CharacterStats.MoveSpeed * 100) * DeltaTime;

	//Travel only the distance needed to reach the destination
	if (TravelVector.Size2D() <= DeltaLocation.Size2D())
	{
		DeltaLocation = TravelVector;
		bHasNextMove = false;
	}

	return DeltaLocation;
}

//Wrapper function to get current grid location
FGridCoordinate AMapPawn::GetActorGridLocation() const
{
	return UGridFunctions::WorldToGridLocation(GetActorLocation());
}

//Validates a move and updates the WorldGrid
bool AMapPawn::IsMoveValid(FVector DeltaLocation) const
{
	TActorIterator<AWorldGrid> GridItr(GetWorld());
	if (GridItr)
	{
		FGridCoordinate CurrentPosition = GetActorGridLocation();
		FGridCoordinate CurrentDestination = UGridFunctions::WorldToGridLocation(GetActorLocation() + DeltaLocation);
		if (CurrentPosition == CurrentDestination || GridItr->MovePawn(CurrentPosition, CurrentDestination))
		{
			return true;
		}
	}

	return false;

}

//Called on CurrentDestination replication (client)
void AMapPawn::OnDestination_Rep()
{
	bMoveCharacter = true;
	BeginMove();
}

//Called on Rotation replication (client)
void AMapPawn::OnRotation_Rep()
{
	bRotateCharacter = true;
}

//sets variables for replicaton over a network
void AMapPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMapPawn, CharacterStats);
	DOREPLIFETIME(AMapPawn, FinalDestination);
	DOREPLIFETIME(AMapPawn, Rotation);
}

//Sets a destination if valid (server)
void AMapPawn::Server_SetDestination_Implementation(FGridCoordinate GridLocation)
{
	TActorIterator<AWorldGrid> GridItr(GetWorld());
	FVector Location = GetActorLocation();
	FVector CurrentDestination = UGridFunctions::GridToWorldLocation(GridLocation);

	if (GridItr)
	{
		TArray<FGridCoordinate> Array;
		bool Success = GridItr->GetPath(GetActorGridLocation(), GridLocation, Array);
		if (Success)
		{
			for (const auto& entry : Array)
			{
				UE_LOG(LogNotice, Warning, TEXT("Path->(%i, %i)"), entry.X, entry.Y);
			}
		}
		else
		{
			UE_LOG(LogNotice, Warning, TEXT("Couldn't find a Path"));
		}

		if (!GridItr->IsOccupied(GridLocation) || GridLocation == GetActorGridLocation())
		{
			FinalDestination.X = CurrentDestination.X;
			FinalDestination.Y = CurrentDestination.Y;
			FinalDestination.Z = Location.Z;

			bMoveCharacter = true;
			BeginMove();
		}
	}

	//Rotate the character;

	FVector TravelVector = CurrentDestination - Location;
	if (TravelVector.Size2D() > 0.1f)
	{
		TravelVector = TravelVector.GetSafeNormal2D();
		float AngleBetween = FMath::Atan2(TravelVector.Y, TravelVector.X);

		AngleBetween = FMath::RadiansToDegrees(AngleBetween);
		FRotator DeltaRotation(0.0f, AngleBetween - 90.0f, 0.0f);

		if (PawnBody)
		{
			Rotation = DeltaRotation;
			bRotateCharacter = true;
		}
	}
}

//Vaidates non-cheat move (server)
bool AMapPawn::Server_SetDestination_Validate(FGridCoordinate WorldLocation)
{
	return true;
}