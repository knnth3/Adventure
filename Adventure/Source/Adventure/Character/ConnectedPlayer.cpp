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

AMapPawn * AConnectedPlayer::GetSelectedPawn() const
{
	return m_SelectedPawn;
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

void AConnectedPlayer::PlayPawnCelebrationAnimation(int AnimationIndex)
{
	if (m_SelectedPawn)
	{
		Server_PlayPawnCelebrationAnimation(AnimationIndex, m_SelectedPawn->GetPawnID(), m_SelectedPawn->GetActorLocation());
	}
}

void AConnectedPlayer::PawnAttack(const FVector& TargetLocation, int AttackIndex)
{
	if (m_SelectedPawn)
	{
		Server_PawnAttack(AttackIndex, m_SelectedPawn->GetPawnID(), m_SelectedPawn->GetActorLocation(), TargetLocation);
	}
}

void AConnectedPlayer::KillPawn()
{
	if (m_SelectedPawn)
	{
		Server_KillPawn(m_SelectedPawn->GetPawnID(), m_SelectedPawn->GetActorLocation());
	}
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
	if (PawnID == -1)
	{
		SetCameraToOverview();
		m_SelectedPawn = nullptr;
	}
	else
	{
		for (TActorIterator<AMapPawn> MapPawnIter(GetWorld()); MapPawnIter; ++MapPawnIter)
		{
			if (MapPawnIter->GetPawnID() == PawnID)
			{
				m_SelectedPawn = *MapPawnIter;
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
		// Calculate Time
		float Time = 0.8f;

		PlayerController->SetViewTargetWithBlend(this, Time, VTBlend_Cubic, 0.0f, true);
		m_CameraType = CONNECTED_PLAYER_CAMERA::OVERVIEW;
		OnCameraTypeChanged(m_CameraType);
	}
}

void AConnectedPlayer::SetCameraToCharacter()
{
	if (m_SelectedPawn)
	{
		m_SelectedPawn->SetFocusToPawn(0.8f);
		m_CameraType = CONNECTED_PLAYER_CAMERA::CHARACTER;
		OnCameraTypeChanged(m_CameraType);
	}

}

//////////////////////// Private Server Functions ////////////////////////

void AConnectedPlayer::Server_KillPawn_Implementation(const int PawnID, const FVector & PawnLocation)
{
	APS_Multiplayer* state = Cast<APS_Multiplayer>(PlayerState);
	TActorIterator<AWorldGrid> WorldGrid(GetWorld());
	if (WorldGrid)
	{
		AMapPawn* pawn = WorldGrid->ServerOnly_GetPawn(PawnLocation, PawnID);
		if (pawn && state && (state->GetGameID() == 0 || state->GetGameID() == pawn->GetOwnerID()))
		{
			pawn->KillPawn();
		}
	}
}

bool AConnectedPlayer::Server_KillPawn_Validate(const int PawnID, const FVector & PawnLocation)
{
	return true;
}

void AConnectedPlayer::Server_PawnAttack_Implementation(int AttackIndex, const int PawnID, const FVector & PawnLocation, const FVector& TargetLocation)
{
	APS_Multiplayer* state = Cast<APS_Multiplayer>(PlayerState);
	TActorIterator<AWorldGrid> WorldGrid(GetWorld());
	if (WorldGrid)
	{
		AMapPawn* pawn = WorldGrid->ServerOnly_GetPawn(PawnLocation, PawnID);
		if (pawn && state && (state->GetGameID() == 0 || state->GetGameID() == pawn->GetOwnerID()))
		{
			pawn->Attack(AttackIndex, TargetLocation);
		}
	}
}

bool AConnectedPlayer::Server_PawnAttack_Validate(int AttackIndex, const int PawnID, const FVector & PawnLocation, const FVector& TargetLocation)
{
	return true;
}

void AConnectedPlayer::Server_MovePlayer_Implementation(const int PawnID, const FVector& Location, const FVector& Destination)
{
	APS_Multiplayer* state = Cast<APS_Multiplayer>(PlayerState);
	TActorIterator<AWorldGrid> WorldGrid(GetWorld());
	if (WorldGrid)
	{
		AMapPawn* pawn = WorldGrid->ServerOnly_GetPawn(Location, PawnID);
		UE_LOG(LogNotice, Warning, TEXT("<ConnectedPlayer>: Finding pawn with id %i : %i"), PawnID, pawn);
		if (pawn && state)
		{
			UE_LOG(LogNotice, Warning, TEXT("<ConnectedPlayer%i>: Attempting to move pawn with owner ID: %i"), state->GetGameID(), pawn->GetOwnerID());
			if (state->GetGameID() == 0 || (state->GetGameID() == pawn->GetOwnerID()))
			{
				pawn->ServerOnly_SetDestination(Destination);
			}
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

void AConnectedPlayer::Server_PlayPawnCelebrationAnimation_Implementation(int AnimationIndex, const int PawnID, const FVector& PawnLocation)
{
	APS_Multiplayer* state = Cast<APS_Multiplayer>(PlayerState);
	TActorIterator<AWorldGrid> WorldGrid(GetWorld());
	if (WorldGrid)
	{
		AMapPawn* pawn = WorldGrid->ServerOnly_GetPawn(PawnLocation, PawnID);
		if (pawn && state && (state->GetGameID() == 0 || state->GetGameID() == pawn->GetOwnerID()))
		{
			pawn->Celebrate(AnimationIndex);
		}
	}
}

bool AConnectedPlayer::Server_PlayPawnCelebrationAnimation_Validate(int AnimationIndex, const int PawnID, const FVector& PawnLocation)
{
	return true;
}
