// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.
#include "ConnectedPlayer.h"

#include "../PlayerControllers/PC_Multiplayer.h"
#include "Grid/WorldGrid.h"
#include "MapPawn.h"
#include "Adventure.h"
#include "GameModes/GM_Multiplayer.h"
#include "GameStates/GS_Multiplayer.h"
#include "PlayerStates/PS_Multiplayer.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AConnectedPlayer::AConnectedPlayer()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bRegistered = false;
	m_SelectedPawn = nullptr;
	SpectatingPawnID = 0;
	m_CameraTransitionAcceleration = 5500.0f;
	m_CameraType = CONNECTED_PLAYER_CAMERA::OVERVIEW;

	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

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
	CameraBoom->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation

	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
}

// Called every frame
void AConnectedPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AConnectedPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

//sets variables for replicaton over a network
void AConnectedPlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AConnectedPlayer, SpectatingPawnID);
}

bool AConnectedPlayer::GetPawnLocation(FVector & Location) const
{
	if (m_SelectedPawn)
	{
		Location = m_SelectedPawn->GetActorLocation();
		return true;
	}
	return false;
}

void AConnectedPlayer::SwapCameraView()
{
	switch (m_CameraType)
	{
	case CONNECTED_PLAYER_CAMERA::CHARACTER:
		SetCameraToOverview();
		break;
	case CONNECTED_PLAYER_CAMERA::OVERVIEW:
		SetCameraToCharacter();
		break;
	case CONNECTED_PLAYER_CAMERA::NONE:
	default:
		UE_LOG(LogNotice, Warning, TEXT("Connected player camera type is invalid"));
		break;
	}
}

void AConnectedPlayer::RotatePawnCameraUpDown(const float & AxisValue, const float & DeltaTime)
{
	if (m_SelectedPawn)
	{
		m_SelectedPawn->RotateCameraPitch(AxisValue, DeltaTime);
	}
}

void AConnectedPlayer::RotatePawnCameraLeftRight(const float & AxisValue, const float & DeltaTime)
{
	if (m_SelectedPawn)
	{
		m_SelectedPawn->RotateCameraYaw(AxisValue, DeltaTime);
	}
}

void AConnectedPlayer::ZoomPawnCameraInOut(const float & AxisValue, const float & DeltaTime)
{
	if (m_SelectedPawn)
	{
		m_SelectedPawn->ZoomCamera(AxisValue, DeltaTime);
	}
}

CONNECTED_PLAYER_CAMERA AConnectedPlayer::GetCameraType() const
{
	return m_CameraType;
}

bool AConnectedPlayer::GetSelectedActorLocation(FVector& Location) const
{
	if (m_SelectedPawn)
	{
		Location = m_SelectedPawn->GetActorLocation();
		return true;
	}

	return false;
}

bool AConnectedPlayer::GetSelectedActorStats(FMapPawnStatSheet & Stats) const
{
	if (m_SelectedPawn)
	{
		Stats = m_SelectedPawn->GetStatSheet();
		return true;
	}

	return false;
}

int AConnectedPlayer::GetSpectatingPawnID() const
{
	return SpectatingPawnID;
}

void AConnectedPlayer::MovePlayer(const FVector & Location)
{
	if (m_SelectedPawn)
	{
		UE_LOG(LogNotice, Warning, TEXT("<Pawn #%i>: Moving Pawn..."), m_SelectedPawn->GetPawnID());
		Server_MovePlayer(m_SelectedPawn->GetPawnID(), m_SelectedPawn->GetActorLocation(), Location);
	}
}

void AConnectedPlayer::SetPawnTargetLocation(const FVector & Location)
{
	Server_SetPawnTargetLocation(0, Location);
}

void AConnectedPlayer::ClearPawnTargetLocation()
{
	Server_ClearPawnTargetLocation();
}

void AConnectedPlayer::SetSpectatingPawn(const int PawnID)
{
	for (TActorIterator<AMapPawn> MapPawnIter(GetWorld()); MapPawnIter; ++MapPawnIter)
	{
		if (MapPawnIter->GetPawnID() == PawnID)
		{
			m_SelectedPawn = *MapPawnIter;

			if (m_CameraType == CONNECTED_PLAYER_CAMERA::CHARACTER)
			{
				SetCameraToCharacter();
			}
		}
	}
}

void AConnectedPlayer::SpawnPawn(FGridCoordinate Location, const int ClassIndex, const int OwnerID)
{
	if (HasAuthority())
	{
		TActorIterator<AWorldGrid> WorldGrid(GetWorld());
		if (WorldGrid)
		{
			WorldGrid->ServerOnly_AddPawn(ClassIndex, Location, OwnerID);
		}
	}
}

void AConnectedPlayer::SetCameraToOverview()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController && m_SelectedPawn)
	{
		// Get Distance between the two cameras
		FVector StartLocation = m_SelectedPawn->GetActorLocation();
		FVector EndLocation = FollowCamera->GetComponentLocation();
		float Distance = FVector::Distance(StartLocation, EndLocation);

		// Calculate Time
		float Time = FMath::Sqrt(2 * Distance / m_CameraTransitionAcceleration);

		PlayerController->SetViewTargetWithBlend(this, Time, VTBlend_Cubic, 0.0f, true);
		m_CameraType = CONNECTED_PLAYER_CAMERA::OVERVIEW;
		OnCameraTypeChanged(m_CameraType);
	}
}

void AConnectedPlayer::SetCameraToCharacter()
{
	if (m_SelectedPawn)
	{
		m_SelectedPawn->SetFocusToPawn(FollowCamera->GetComponentLocation(), m_CameraTransitionAcceleration);
		m_CameraType = CONNECTED_PLAYER_CAMERA::CHARACTER;
		OnCameraTypeChanged(m_CameraType);
	}

}

//////////////////////// Private Server Functions ////////////////////////

void AConnectedPlayer::Server_MovePlayer_Implementation(const int PawnID, const FVector& Location, const FVector& Destination)
{
	APS_Multiplayer* state = Cast<APS_Multiplayer>(PlayerState);
	TActorIterator<AWorldGrid> WorldGrid(GetWorld());
	if (WorldGrid)
	{
		AMapPawn* pawn = WorldGrid->ServerOnly_GetPawn(Location, PawnID);
		if (pawn && state && (state->GetGameID() == 0 || state->GetGameID() == pawn->GetOwnerID()))
		{
			pawn->ServerOnly_SetDestination(Destination);
		}
	}
}

bool AConnectedPlayer::Server_MovePlayer_Validate(const int PawnID, const FVector& Location, const FVector& Destination)
{
	return true;
}

void AConnectedPlayer::Server_SetPawnTargetLocation_Implementation(const int PawnID, const FVector& Location)
{
}

bool AConnectedPlayer::Server_SetPawnTargetLocation_Validate(const int PawnID, const FVector& Location)
{
	return true;
}

void AConnectedPlayer::Server_ClearPawnTargetLocation_Implementation()
{
}

bool AConnectedPlayer::Server_ClearPawnTargetLocation_Validate()
{
	return true;
}