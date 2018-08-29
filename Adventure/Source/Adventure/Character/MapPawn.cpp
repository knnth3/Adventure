// Fill out your copyright notice in the Description page of Project Settings.
#include "MapPawn.h"

#include "Grid/WorldGrid.h"
#include "Adventure.h"
#include "MapPawnComponent_Head.h"

// Sets default values
AMapPawn::AMapPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bMoveCharacter = false;
	bReplicates = true;
	bRotateCharacter = false;
	bHasTarget = false;
	OwnerID = -1;
	SkeletalMeshIndex = -1;

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
	PawnBody->ComponentTags.Add(FName("Outline"));

	PawnHeadC = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pawn_Head"));
	PawnHeadC->SetupAttachment(PawnBody);
	PawnHeadC->ComponentTags.Add(FName("Outline"));

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetRelativeTransform(FTransform(FVector(0,0,200)));
	CameraBoom->SetupAttachment(Scene);
	CameraBoom->TargetArmLength = DesiredCameraZoom = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = false; // Rotate the arm based on the controller
	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->CameraRotationLagSpeed = 10.0f;
	CameraBoom->bDoCollisionTest = false;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
}

//sets variables for replicaton over a network
void AMapPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMapPawn, StatSheet);
	DOREPLIFETIME(AMapPawn, FinalDestination);
	DOREPLIFETIME(AMapPawn, Rotation);
	DOREPLIFETIME(AMapPawn, OwnerID);
	DOREPLIFETIME(AMapPawn, PawnID);
	DOREPLIFETIME(AMapPawn, TargetedLocation);

}

// Called when the game starts or when spawned
void AMapPawn::BeginPlay()
{
	Super::BeginPlay();
	
	FinalDestination = GetActorLocation();

	if (PawnBody)
	{
		// Location, Rotation, Scale, bWeld
		FAttachmentTransformRules rules(
			EAttachmentRule::SnapToTarget,
			EAttachmentRule::KeepWorld,
			EAttachmentRule::KeepRelative,
			true
		);

		// Body Set-up
		PawnBody->SetWorldScale3D(FVector(0.5f));
		PawnBody->SetSkeletalMesh(BaseSkeletalMesh);
		PawnBody->SetAnimInstanceClass(BaseAnimationBlueprint->GetAnimBlueprintGeneratedClass());

		// Head Set-up
		FTransform Transform;
		Transform.SetScale3D(FVector(0.0031f));
		PawnHeadC->SetWorldTransform(Transform);
		PawnHeadC->SetStaticMesh(BaseHeadMesh);
		PawnHeadC->AttachToComponent(PawnBody, rules, "biped_mr_Neck_jnt");
	}

	if (HasAuthority())
	{
		FRandomStream rand;
		rand.GenerateNewSeed();
		StatSheet.Name = StatSheet.Name + FString::FromInt(rand.FRandRange(0, 100));
	}
}

void AMapPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (PawnHead)
	{
		PawnHead->Destroy();
	}
}

// Called every frame
void AMapPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MoveToNextLocation();
	MovePawn(DeltaTime);
	RotatePawn(DeltaTime);

	// Zoom camera using lerp
	if (CameraBoom->TargetArmLength != DesiredCameraZoom)
	{
		CameraBoom->TargetArmLength = FMath::Lerp(CameraBoom->TargetArmLength, DesiredCameraZoom, 0.075f);
	}
}

// Called to bind functionality to input
void AMapPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AMapPawn::RotateCameraPitch(const float & AxisValue, const float & DeltaTime)
{
	if (AxisValue)
	{
		FTransform transform = CameraBoom->GetRelativeTransform();
		FRotator rotation = transform.GetRotation().Rotator();
		float pitch = rotation.Pitch - AxisValue * DeltaTime * CameraSettings.AngularVelocity;
		pitch = FMath::ClampAngle(pitch, CameraSettings.MaxUpRotation, CameraSettings.MaxDownRotation);

		CameraBoom->SetRelativeRotation(FRotator(pitch, rotation.Yaw, rotation.Roll));
	}
}

void AMapPawn::RotateCameraYaw(const float & AxisValue, const float & DeltaTime)
{
	if (AxisValue)
	{
		FTransform transform = CameraBoom->GetRelativeTransform();
		FRotator rotation = transform.GetRotation().Rotator();
		float yaw = rotation.Yaw - AxisValue * DeltaTime * CameraSettings.AngularVelocity;

		CameraBoom->SetRelativeRotation(FRotator(rotation.Pitch, yaw, rotation.Roll));
	}
}

void AMapPawn::ZoomCamera(const float & AxisValue, const float & DeltaTime)
{
	if (AxisValue)
	{
		DesiredCameraZoom = FMath::Clamp(
			DesiredCameraZoom - (AxisValue * DeltaTime * CameraSettings.ZoomSpeed),
			CameraSettings.MaxInZoom, CameraSettings.MaxOutZoom
		);
	}
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
		if (Success)
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

void AMapPawn::SetTarget(FVector Location)
{
	Server_TargetLocation(Location);
}

void AMapPawn::ClearTarget()
{
	Server_ClearTarget();
}

//Returns the pawns stats
FMapPawnStatSheet AMapPawn::GetStatSheet() const
{
	return StatSheet;
}

//Return true if the pawn is moving
bool AMapPawn::IsMoving() const
{
	return bMoveCharacter;
}

void AMapPawn::Attack(const AMapPawnAttack * AttackMove, const FVector Location)
{
	if (AttackMove)
	{
		AttackMove->Attack(StatSheet, GetActorLocation(), Location);
	}
}

void AMapPawn::SetAsActivePawn(bool Active)
{
	if(Active != bIsTurnActive)
	{
		bIsTurnActive = Active;
		if (Active)
			OnBeginTurn();
		else
			OnEndTurn();
	}
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

void AMapPawn::ScaleHead(const FVector & Scale)
{
	PawnHeadC->SetWorldScale3D(Scale);
}

//Moves a pawn if its destination is not the same as its position
void AMapPawn::MovePawn(float DeltaTime)
{
	if (bMoveCharacter)
	{
		if (HasAuthority())
		{
			FVector DeltaLocation = GetNextMove(DeltaTime, bMoveCharacter);

			//On the server, check if the delta location is valid
			//If so, move the pawn one space back
			if (IsMoveValid(DeltaLocation))
			{
				AddActorWorldOffset(DeltaLocation);
			}
			if (!bMoveCharacter && MoveQueue.empty() && bHasTarget)
			{
				Server_TargetLocation(TargetedLocation);
			}
		}
		else
		{
			//Clients can do the normal work based off the info recieved from the server
			FVector DeltaLocation = GetNextMove(DeltaTime, bMoveCharacter);
			AddActorWorldOffset(DeltaLocation);
		}
	}
}

//Rotates a pawn if the bRotateCharacter was set (usually when setDestination was called)
void AMapPawn::RotatePawn(float DeltaTime)
{
	if (PawnBody && bRotateCharacter)
	{
		float Multiplier = StatSheet.TurnSpeed;
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
	FVector DeltaLocation = TravelVector.GetSafeNormal() * (StatSheet.MoveSpeed * (100.0f/6.0f)) * DeltaTime;

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

void AMapPawn::Server_TargetLocation_Implementation(FVector ViewLocation)
{
	FVector CurrentLocation = GetActorLocation();

	//Rotate the character;
	FVector TravelVector = ViewLocation - CurrentLocation;
	if (TravelVector.Size2D() > 0.1f)
	{
		TargetedLocation = ViewLocation;
		TravelVector = TravelVector.GetSafeNormal2D();
		float AngleBetween = FMath::Atan2(TravelVector.Y, TravelVector.X);

		AngleBetween = FMath::RadiansToDegrees(AngleBetween);
		FRotator FinalRotation(0.0f, AngleBetween - 90.0f, 0.0f);

		if (PawnBody)
		{
			bHasTarget = true;
			Rotation = FinalRotation;
			bRotateCharacter = true;
		}
	}
}

bool AMapPawn::Server_TargetLocation_Validate(FVector Location)
{
	return true;
}

void AMapPawn::Server_ClearTarget_Implementation()
{
	bHasTarget = false;
	TargetedLocation = FVector(0.0f);
}

bool AMapPawn::Server_ClearTarget_Validate()
{
	return true;
}


//Called on CurrentDestination replication (client)
void AMapPawn::OnDestination_Rep()
{
	bMoveCharacter = true;
}

//Called on Rotation replication (client)
void AMapPawn::OnRotation_Rep()
{
	bRotateCharacter = true;
}

//Sets a destination if valid (server)
void AMapPawn::Server_SetDestination_Implementation(FGridCoordinate GridLocation)
{
	TActorIterator<AWorldGrid> GridItr(GetWorld());
	FVector Location = GetActorLocation();
	FVector CurrentDestination = UGridFunctions::GridToWorldLocation(GridLocation);

	if (GridItr)
	{
		//Move the character
		if (!GridItr->IsSpaceTraversable(GridLocation) || GridLocation == GetActorGridLocation())
		{
			FinalDestination.X = CurrentDestination.X;
			FinalDestination.Y = CurrentDestination.Y;
			FinalDestination.Z = Location.Z;

			bMoveCharacter = true;
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

void AMapPawn::Server_Attack_Implementation(FVector Location)
{

}

bool AMapPawn::Server_Attack_Validate(FVector Location)
{
	return true;
}