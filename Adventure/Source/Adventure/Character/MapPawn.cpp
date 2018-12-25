// Fill out your copyright notice in the Description page of Project Settings.
#include "MapPawn.h"

#include "Grid/WorldGrid.h"
#include "Adventure.h"
#include "MapPawnComponent_Head.h"
#include "Grid/PathFinder.h"
#include "Components/StatisticsComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/HoverArrowComponent.h"
#include "Components/InteractionInterfaceComponent.h"

// Sets default values
AMapPawn::AMapPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicateMovement = false;
	bWasPathingSucessfull = false;
	bRotatePawn = false;
	bHasTarget = false;
	m_CurrentRequest = nullptr;
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

	//Create the base scene component
	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Focus"));
	RootComponent = Scene;

	// Create a Statistics Component
	Statistics = CreateDefaultSubobject<UStatisticsComponent>(TEXT("Pawn Stats"));

	// Create an Inventory Component
	Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
	Inventory->AttatchStatistics(Statistics);

	// Create an Attack Interface Component
	InteractionInterface = CreateDefaultSubobject<UInteractionInterfaceComponent>(TEXT("Interaction Interface"));

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
		Statistics->SetCharacterName(TEXT("Alfie"));
		m_PawnID = GetNewID();
	}
}

void AMapPawn::ServerOnly_SetClassIndex(const int Index)
{
	m_ClassIndex = Index;
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

bool AMapPawn::IsMoving() const
{
	return Statistics->GetCurrentAction() == PAWN_ACTION::MOVING;
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
		if (CameraBoom->IsCollisionFixApplied())
		{
			FVector BoomLocation = CameraBoom->GetComponentLocation();
			FVector CameraLocation = FollowCamera->GetComponentLocation();
			float distance = FVector::Dist(BoomLocation, CameraLocation);

			float value = FMath::Min(distance, distance - (AxisValue * DeltaTime * m_CameraSettings.ZoomSpeed));

			m_CameraSettings.FinalCameraZoom = value;
			CameraBoom->TargetArmLength = distance;
		}
		else
		{
			m_CameraSettings.FinalCameraZoom = FMath::Clamp(
				m_CameraSettings.FinalCameraZoom - (AxisValue * DeltaTime * m_CameraSettings.ZoomSpeed),
				m_CameraSettings.MaxInZoom, m_CameraSettings.MaxOutZoom
			);
		}
	}
}

void AMapPawn::ServerOnly_SetDestination(const FVector & Destination)
{
	if (HasAuthority() && (Statistics->GetCurrentAction() == PAWN_ACTION::NONE || Statistics->GetCurrentAction() == PAWN_ACTION::MOVING))
	{
		FVector CurrentLocation = GetActorLocation();
		if (CurrentLocation != Destination)
		{
			if (m_CurrentRequest)
			{
				m_CurrentRequest->CancelRequest();
				m_CurrentRequest = nullptr;
			}

			// b = c- a
			FVector TravelVec = Destination - CurrentLocation;
			FVector EndLocation = Destination;
			if (TravelVec.Size() > (TRACE_RADIUS - 1) * CELL_STEP)
			{
				TravelVec.Normalize();
				EndLocation = CurrentLocation + (TravelVec * (TRACE_RADIUS - 1) * CELL_STEP);
			}

			FPathFoundDelegate Callback;
			Callback.BindUObject(this, &AMapPawn::OnNewPathRecieved);

			m_FullDestination = Destination;
			m_CurrentRequest = FPathFinder::RequestFindPath(GetActorLocation(), EndLocation, GetWorld(), Callback);
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

UStatisticsComponent * AMapPawn::GetStats()const
{
	return Statistics;
}

UInventoryComponent * AMapPawn::GetInventory()const
{
	return Inventory;
}

UInteractionInterfaceComponent * AMapPawn::GetInteractionInterface()const
{
	return InteractionInterface;
}

void AMapPawn::RotatePawn(float DeltaTime)
{
	if (bRotatePawn && Statistics->GetCurrentStatusEffect() != STATUS_EFFECT::FROZEN)
	{
		// Rotate towards path
		FVector currentForward = GetActorForwardVector();
		FVector finalForward = m_ForwardVector;

		float dot = FVector::DotProduct(currentForward, finalForward);
		float mag = FVector::Distance(currentForward, FVector(0.0f)) * FVector::Distance(m_ForwardVector, FVector(0.0f));
		float angle = FMath::RadiansToDegrees(FMath::Acos(dot / mag));

		float w = DeltaTime * Statistics->GetMoveSpeed() * 0.5;

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
	if (Statistics->GetCurrentStatusEffect() == STATUS_EFFECT::FROZEN)
	{
		if (!m_MoveQueue.empty())
		{
			m_MoveQueue.clear();
		}
	}
	else if (Statistics->GetCurrentAction() != PAWN_ACTION::MOVING)
	{
		if (m_Destination != GetActorLocation())
		{
			m_MoveQueue.clear();
			m_Destination = GetActorLocation();
		}
	}
	else
	{
		FVector currentLocation = GetActorLocation();
		FVector finalLocation = m_Destination;
		FVector TravelVector = finalLocation - currentLocation;

		float moveSpeed = Statistics->GetMoveSpeed();
		float v = DeltaTime * Conversions::Feet::ToCentimeters(moveSpeed) * 0.5;
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
				FGridCoordinate cur = currentLocation;
				FGridCoordinate fin = m_FullDestination;
				if (bWasPathingSucessfull && cur != fin)
				{
					// UE_LOG(LogNotice, Warning, TEXT("<Pawn_%i>: Attempting to finish pathing [ (%i, %i) to (%i, %i) ]"), GetPawnID(), fin.X, fin.Y, cur.X, cur.Y);
					ServerOnly_SetDestination(m_FullDestination);
				}
				else
				{
					// UE_LOG(LogNotice, Warning, TEXT("<Pawn_%i>: Pathing has reached the final location!"), GetPawnID());
					m_FullDestination = currentLocation;
				}
				Statistics->SetCurrentAction(PAWN_ACTION::NONE);
			}
		}
	}
}

void AMapPawn::OnNewPathRecieved(bool success, TArray<FVector> Path)
{
	// Make sure to let go of the request
	m_CurrentRequest = nullptr;
	bWasPathingSucessfull = success;
	if (success)
	{
		m_MoveQueue.clear();
		for (auto& step : Path)
		{
			m_MoveQueue.push_back(step);
		}

		FVector newDestination = m_MoveQueue.front();
		m_MoveQueue.pop_front();
		Multicast_SetActiveDestination(newDestination);

	}
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

	Statistics->SetCurrentAction(PAWN_ACTION::MOVING);
	bRotatePawn = true;
}