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
	bReplicateMovement = false;
	bPlayCelebrationAnim = false;
	bAttacking = false;
	bMovePawn = false;
	bRotatePawn = false;
	bHasTarget = false;
	bIsFrozen = false;
	m_PawnID = -1;
	m_CelebrationAnimIndex = 0;
	m_AttackAnimIndex = 0;

	FAttachmentTransformRules rules(
		EAttachmentRule::SnapToTarget,
		EAttachmentRule::KeepWorld,
		EAttachmentRule::KeepWorld,
		true
	);

	m_CameraSettings.AngularVelocity = 240.0f;
	m_CameraSettings.ZoomSpeed = Conversions::Meters::ToCentimeters(50.0f);
	m_CameraSettings.MaxUpRotation = -85.0f;
	m_CameraSettings.MaxDownRotation = -25.0f;
	m_CameraSettings.MaxOutZoom = Conversions::Meters::ToCentimeters(8.0f);
	m_CameraSettings.MaxInZoom = Conversions::Meters::ToCentimeters(2.0f);

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
	CameraBoom->SetRelativeTransform(FTransform(FVector(0,0,200)));
	CameraBoom->SetupAttachment(Scene);
	CameraBoom->TargetArmLength = m_CameraSettings.FinalCameraZoom = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = false; // Rotate the arm based on the controller
	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->CameraRotationLagSpeed = 10.0f;
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->bAbsoluteRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Create a selection arrow
	ArrowComponent = CreateDefaultSubobject<UHoverArrowComponent>(TEXT("Selection Arrow"));
	ArrowComponent->SetupAttachment(Scene);
	ArrowComponent->SetIsReplicated(true);
}

//sets variables for replicaton over a network
void AMapPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMapPawn, m_StatSheet);
	DOREPLIFETIME(AMapPawn, m_PawnID);
	DOREPLIFETIME(AMapPawn, m_OwnerID);

}

// Called when the game starts or when spawned
void AMapPawn::BeginPlay()
{
	Super::BeginPlay();
	
	m_ForwardVector = GetActorForwardVector();
	m_Destination = GetActorLocation();

	if (HasAuthority())
	{
		FRandomStream rand;
		rand.GenerateNewSeed();
		m_StatSheet.Name = m_StatSheet.Name + FString::FromInt(rand.FRandRange(0, 100));
		m_PawnID = GetNewID();
	}
}

int AMapPawn::GetClassIndex_Implementation() const
{
	return m_ClassIndex;
}

int AMapPawn::GetObjectID_Implementation() const
{
	return GetPawnID();
}

void AMapPawn::ServerOnly_SetClassIndex(const int Index)
{
	m_ClassIndex = Index;
}

void AMapPawn::OnCelebrationAnimEnd()
{
	bPlayCelebrationAnim = false;
}

void AMapPawn::OnAttackAnimEnd()
{
	bAttacking = false;
}

void AMapPawn::ServerOnly_SetOwnerID(const int ID)
{
	UE_LOG(LogNotice, Warning, TEXT("<Pawn_%i>: New owner ID set to %i"), GetPawnID(), ID);

	m_OwnerID = ID;
}

// Called every frame
void AMapPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotatePawn(DeltaTime);
	MovePawn(DeltaTime);

	// Zoom camera using lerp
	if (CameraBoom->TargetArmLength != m_CameraSettings.FinalCameraZoom)
	{
		CameraBoom->TargetArmLength = FMath::Lerp(CameraBoom->TargetArmLength, m_CameraSettings.FinalCameraZoom, 0.075f);
	}
}

// Called to bind functionality to input
void AMapPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

int AMapPawn::GetPawnID() const
{
	return m_PawnID;
}

int AMapPawn::GetOwnerID() const
{
	return m_OwnerID;
}

const FVector AMapPawn::GetCameraLocation() const
{
	return FollowCamera->GetComponentLocation();
}

FMapPawnStatSheet AMapPawn::GetStatSheet() const
{
	return m_StatSheet;
}

bool AMapPawn::IsMoving() const
{
	return bMovePawn;
}

bool AMapPawn::IsCelebrating() const
{
	return bPlayCelebrationAnim;
}

bool AMapPawn::IsAttacking() const
{
	return bAttacking;
}

bool AMapPawn::IsDead() const
{
	return bIsDead;
}

void AMapPawn::Celebrate(int AnimationIndex)
{
	if (!IsMoving() && !bIsFrozen && !bPlayCelebrationAnim && !bAttacking)
	{
		Multicast_Celebrate(AnimationIndex);
	}
}

void AMapPawn::Attack(int AttackIndex, const FVector& TargetLocation)
{
	if (!IsMoving() && !bIsFrozen && !bPlayCelebrationAnim && !bAttacking)
	{
		Multicast_Attack(AttackIndex, TargetLocation);
	}
}

void AMapPawn::KillPawn()
{
	Multicast_KillPawn();
}

void AMapPawn::RotateCameraPitch(const float & AxisValue, const float & DeltaTime)
{
	if (AxisValue)
	{
		FTransform transform = CameraBoom->GetRelativeTransform();
		FRotator rotation = transform.GetRotation().Rotator();
		float pitch = rotation.Pitch - AxisValue * DeltaTime * m_CameraSettings.AngularVelocity;
		pitch = FMath::ClampAngle(pitch, m_CameraSettings.MaxUpRotation, m_CameraSettings.MaxDownRotation);

		CameraBoom->SetRelativeRotation(FRotator(pitch, rotation.Yaw, rotation.Roll));
	}
}

void AMapPawn::RotateCameraYaw(const float & AxisValue, const float & DeltaTime)
{
	if (AxisValue)
	{
		FTransform transform = CameraBoom->GetRelativeTransform();
		FRotator rotation = transform.GetRotation().Rotator();
		float yaw = rotation.Yaw - AxisValue * DeltaTime * m_CameraSettings.AngularVelocity;

		CameraBoom->SetRelativeRotation(FRotator(rotation.Pitch, yaw, rotation.Roll));
	}
}

void AMapPawn::ZoomCamera(const float & AxisValue, const float & DeltaTime)
{
	if (AxisValue)
	{
		m_CameraSettings.FinalCameraZoom = FMath::Clamp(
			m_CameraSettings.FinalCameraZoom - (AxisValue * DeltaTime * m_CameraSettings.ZoomSpeed),
			m_CameraSettings.MaxInZoom, m_CameraSettings.MaxOutZoom
		);
	}
}

void AMapPawn::ServerOnly_SetDestination(const FGridCoordinate & Destination)
{
	if (HasAuthority() && !bIsDead && !bIsFrozen && !bAttacking && !bPlayCelebrationAnim)
	{
		FGridCoordinate CurrentLocation = UGridFunctions::WorldToGridLocation(GetActorLocation());
		if (CurrentLocation != Destination)
		{
			TActorIterator<AWorldGrid> WorldGridItr(GetWorld());
			if (WorldGridItr)
			{
				TArray<FGridCoordinate> Path;
				if (WorldGridItr->ServerOnly_GetPath(CurrentLocation, Destination, Path, GetPawnID()))
				{
					m_MoveQueue.clear();
					for (auto& step : Path)
					{
						m_MoveQueue.push_back(step);
					}

					FVector newDestination = UGridFunctions::GridToWorldLocation(m_MoveQueue.front());
					m_MoveQueue.pop_front();
					Multicast_SetActiveDestination(newDestination);
				}
				else
				{
					UE_LOG(LogNotice, Warning, TEXT("<Pawn_%i Movement>: No path found to (%i, %i)"), GetPawnID(), Destination.X, Destination.Y);
				}
			}
		}
	}
}

void AMapPawn::ServerOnly_SetTargetLocation(const FVector& Location)
{
	m_TargetedLocation = Location;
	FVector newForward = (m_TargetedLocation - GetActorLocation());
	if (newForward != FVector(0.0f))
	{
		m_ForwardVector = newForward;
		m_ForwardVector.Z = 0.0f;
		m_ForwardVector.Normalize();
		bRotatePawn = true;
	}
}

void AMapPawn::ServerOnly_SetStatusEffect(int EffectID)
{
	m_StatSheet.StatusEffect = EffectID;
	Multicast_ApplyNewStatus(EffectID);
}

void AMapPawn::ServerOnly_SetEquipedWeaponType(WEAPON_TYPE type)
{
	m_StatSheet.EquipedWeaponType = type;
}

FVector AMapPawn::ServerOnly_GetDesiredForwardVector() const
{
	return m_ForwardVector;
}

void AMapPawn::SetFocusToPawn(float TransitionTime)
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController)
	{
		// Calculate Time
		PlayerController->SetViewTargetWithBlend(this, TransitionTime, VTBlend_Cubic, 0.0f, true);
	}
}

void AMapPawn::RotatePawn(float DeltaTime)
{
	if (bRotatePawn && !bIsFrozen)
	{
		// Rotate towards path
		FVector currentForward = GetActorForwardVector();
		FVector finalForward = m_ForwardVector;

		float dot = FVector::DotProduct(currentForward, finalForward);
		float mag = FVector::Distance(currentForward, FVector(0.0f)) * FVector::Distance(m_ForwardVector, FVector(0.0f));
		float angle = FMath::RadiansToDegrees(FMath::Acos(dot / mag));

		float w = DeltaTime * GetStatSheet().MoveSpeed * 0.5;

		if (angle > 1.0f)
		{
			FRotator currentAngle = currentForward.Rotation();
			FRotator finalAngle = m_ForwardVector.Rotation();

			FQuat newRotation = FQuat::FastLerp(currentAngle.Quaternion(), finalAngle.Quaternion(), w);

			SetActorRotation(newRotation);
		}
		else if (angle <= 1.0f)
		{
			SetActorRotation(m_ForwardVector.Rotation());
			bRotatePawn = false;
		}
	}
}

void AMapPawn::MovePawn(float DeltaTime)
{
	if (bIsDead)
	{
		if (m_Destination != GetActorLocation())
		{
			m_MoveQueue.clear();
			m_Destination = GetActorLocation();
		}
	}
	else if (bIsFrozen)
	{
		if (!m_MoveQueue.empty())
		{
			m_MoveQueue.clear();
		}
	}
	else if (bMovePawn)
	{
		FVector currentLocation = GetActorLocation();
		FVector finalLocation = m_Destination;
		FVector TravelVector = finalLocation - currentLocation;

		float v = DeltaTime * Conversions::Feet::ToCentimeters(GetStatSheet().MoveSpeed) * 0.5;
		float totalLeft = FVector::Distance(TravelVector, FVector(0.0f));

		if (totalLeft > v)
		{
			TravelVector.Normalize();
			AddActorWorldOffset(TravelVector* v);
		}
		else if (totalLeft <= v)
		{
			SetActorLocation(finalLocation);

			if (HasAuthority() && !m_MoveQueue.empty())
			{
				FVector newDestination = UGridFunctions::GridToWorldLocation(m_MoveQueue.front());
				m_MoveQueue.pop_front();
				Multicast_SetActiveDestination(newDestination);
			}
			else
			{
				bMovePawn = false;
			}
		}
	}
}

void AMapPawn::Multicast_ApplyNewStatus_Implementation(int StatusID)
{
	OnStatusChanged(StatusID);
}

void AMapPawn::Multicast_Celebrate_Implementation(int AnimationIndex)
{
	bPlayCelebrationAnim = true;
	m_CelebrationAnimIndex = AnimationIndex;
}

void AMapPawn::Multicast_Attack_Implementation(int AttackIndex, const FVector& TargetLocation)
{
	bAttacking = true;
	m_AttackAnimIndex = AttackIndex;
	OnAttackInitiated(AttackIndex, TargetLocation);
}

void AMapPawn::Multicast_KillPawn_Implementation()
{
	bIsDead = true;
	OnPawnKilled();
}

int AMapPawn::GetNewID()
{
	static int PawnIDCount = 0;

	return PawnIDCount++;
}

void AMapPawn::Multicast_SetActiveDestination_Implementation(const FVector& Location)
{
	m_Destination = Location;

	m_ForwardVector = (m_Destination - GetActorLocation());
	m_ForwardVector.Z = 0.0f;
	m_ForwardVector.Normalize();

	bMovePawn = true;
	bRotatePawn = true;
}