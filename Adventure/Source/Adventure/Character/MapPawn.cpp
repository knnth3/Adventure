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
	OwnerID = -1;

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

void AMapPawn::RotateUpDown_Implementation(const float & AxisValue, const float & DeltaTime)
{
}

void AMapPawn::RotateLeftRight_Implementation(const float & AxisValue, const float & DeltaTime)
{
}

void AMapPawn::ZoomInOut_Implementation(const float & AxisValue, const float & DeltaTime)
{
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

	//TActorIterator<AWorldGrid> GridItr(GetWorld());
	//if (GridItr)
	//{
	//	GridItr->RemoveActorFromPlay(GetActorGridLocation());
	//}
}

// Called every frame
void AMapPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MoveToNextLocation();
	MovePawn(DeltaTime);
	RotatePawn(DeltaTime);

	FString HealthValue = "Health: " + FString::FromInt(CharacterStats.Health);
	FString IsMoving = "Moving: " + FString::FromInt(bMoveCharacter);

	GetStringOf(Role);
}

// Called to bind functionality to input
void AMapPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

//Set the pawns destination location
void AMapPawn::SetDestination(FGridCoordinate GridLocation)
{
	TActorIterator<AWorldGrid> GridItr(GetWorld());
	if (GridItr)
	{
		//Attempt to find a new path to where we want to go
		TArray<FGridCoordinate> Array;
		bool Success = GridItr->FindPath(UGridFunctions::WorldToGridLocation(FinalDestination), GridLocation, Array);
		if (!Success)
		{
			UE_LOG(LogNotice, Warning, TEXT("Couldn't find a Path"));
			Server_SetDestination(GridLocation, true);
		}
		else
		{
			//Clear the previous queue;
			std::queue<FGridCoordinate> PrevMoves;
			std::swap(MoveQueue, PrevMoves);

			//Add the new path
			for (const auto& entry : Array)
			{
				MoveQueue.push(entry);
			}

			MoveToNextLocation();
		}
	}
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

int AMapPawn::GetOwnerID() const
{
	return OwnerID;
}

void AMapPawn::SetOwnerID(const int ID)
{
	if (HasAuthority())
	{
		OwnerID = ID;
	}
}

int AMapPawn::GetPawnID() const
{
	return PawnID;
}

void AMapPawn::SetPawnID(const int ID)
{
	if (HasAuthority())
	{
		PawnID = ID;
	}
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
			//If so, move the pawn one space back
			if (IsMoveValid(DeltaLocation))
			{
				AddActorWorldOffset(DeltaLocation);
			}
			//else
			//{
			//	SetDestination(UGridFunctions::GridToWorldLocation(GetActorGridLocation()));
			//}
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
		if (CurrentPosition == CurrentDestination || GridItr->SetPosition(CurrentPosition, CurrentDestination))
		{
			return true;
		}
	}

	return false;

}

void AMapPawn::MoveToNextLocation()
{
	if (!bMoveCharacter)
	{
		if (!MoveQueue.empty())
		{
			//Set the destination to the closest move
			Server_SetDestination(MoveQueue.front());
			MoveQueue.pop();
		}
	}
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
	DOREPLIFETIME(AMapPawn, OwnerID);
	DOREPLIFETIME(AMapPawn, PawnID);
	
}

//Sets a destination if valid (server)
void AMapPawn::Server_SetDestination_Implementation(FGridCoordinate GridLocation, bool bRotateOnly)
{
	TActorIterator<AWorldGrid> GridItr(GetWorld());
	FVector Location = GetActorLocation();
	FVector CurrentDestination = UGridFunctions::GridToWorldLocation(GridLocation);

	if (GridItr && !bRotateOnly)
	{
		//Move the character
		if (!GridItr->IsSpaceTraversable(GridLocation) || GridLocation == GetActorGridLocation())
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
bool AMapPawn::Server_SetDestination_Validate(FGridCoordinate WorldLocation, bool bRotateOnly)
{
	return true;
}