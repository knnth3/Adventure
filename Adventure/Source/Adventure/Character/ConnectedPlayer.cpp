// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.
#include "ConnectedPlayer.h"

#include "Grid/WorldGrid.h"
#include "MapPawn.h"
#include "Adventure.h"
#include "GameModes/GM_Multiplayer.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AConnectedPlayer::AConnectedPlayer()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PlayerType = CONNECTED_PLAYER_TYPE::NONE;
	CameraType = CONNECTED_PLAYER_CAMERA::OVERVIEW;
	SelectedMapPawn = nullptr;

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
	static bool bRegistered = false;
	Super::Tick(DeltaTime);

	// Only happens on the server
	if (HasAuthority())
	{
		if (!bRegistered)
		{
			bRegistered = true;
			TActorIterator<AWorldGrid> WorldGrid(GetWorld());
			if (WorldGrid)
			{
				int CharacterID = 0;
				WorldGrid->RegisterPlayerController(this);
				int NewPawn = WorldGrid->AddCharacter(GetPlayerID());
				if (NewPawn)
				{
					SpectatingPawnID = NewPawn;
					OwningPawns.push_back(NewPawn);
					SelectedMapPawn = WorldGrid->GetPawn(NewPawn);
				}
			}
		}
	}
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

	DOREPLIFETIME(AConnectedPlayer, PlayerType);
	DOREPLIFETIME(AConnectedPlayer, SpectatingPawnID);
}

void AConnectedPlayer::ClientChangeState_Implementation(const TURN_BASED_STATE CurrentState)
{
	OnPlayerStatusChanged(CurrentState);
	switch (CurrentState)
	{
	case TURN_BASED_STATE::FREE_ROAM:
		OnEndCombat();
		break;
	case TURN_BASED_STATE::STANDBY:
		OnEndCombat();
		break;
	case TURN_BASED_STATE::ACTIVE:
		OnBeginCombat();
		break;
	default:
		break;
	}
}

int AConnectedPlayer::GetPlayerID() const
{
	if (PlayerState)
	{
		return PlayerState->PlayerId;
	}

	return -1;
}

bool AConnectedPlayer::GetPawnLocation(FVector & Location) const
{
	if (SelectedMapPawn)
	{
		Location = SelectedMapPawn->GetActorLocation();
		return true;
	}
	return false;
}

void AConnectedPlayer::ServerScaleHead_Implementation(const FVector & Size)
{
	if (SelectedMapPawn)
	{
		SelectedMapPawn->ScaleHead(Size);
	}
}

bool AConnectedPlayer::ServerScaleHead_Validate(const FVector & Size)
{
	return true;
}

// Called when the game starts or when spawned
void AConnectedPlayer::BeginPlay()
{
	Super::BeginPlay();
}

void AConnectedPlayer::MovePlayer(const FVector & Location, const int PawnID)
{
	ServerMovePlayer(Location, PawnID);
}

void AConnectedPlayer::Attack(AMapPawnAttack* Attack, const FVector & EndLocation)
{
	if (SelectedMapPawn)
	{
		SelectedMapPawn->Attack(Attack, EndLocation);
	}
}

void AConnectedPlayer::SwapCameraView(const float& time)
{
	if (SelectedMapPawn)
	{
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
		if (PlayerController)
		{
			switch (CameraType)
			{
			case CONNECTED_PLAYER_CAMERA::CHARACTER:
				PlayerController->SetViewTargetWithBlend(this, time, VTBlend_Linear, 0.0f, true);
				CameraType = CONNECTED_PLAYER_CAMERA::OVERVIEW;
				break;
			case CONNECTED_PLAYER_CAMERA::OVERVIEW:
				PlayerController->SetViewTargetWithBlend(SelectedMapPawn, time, VTBlend_Linear, 0.0f, true);
				CameraType = CONNECTED_PLAYER_CAMERA::CHARACTER;
				break;
			case CONNECTED_PLAYER_CAMERA::NONE:
			default:
				UE_LOG(LogNotice, Warning, TEXT("Connected player camera type was invalid"));
				break;
			}
		}
		else
		{
			UE_LOG(LogNotice, Warning, TEXT("No controller found while swapping cameras"));
		}
	}
	else
	{
		UE_LOG(LogNotice, Warning, TEXT("No Map pawn found while swapping cameras"));
	}
}

void AConnectedPlayer::RotatePawnCameraUpDown(const float & AxisValue, const float & DeltaTime)
{
	if (SelectedMapPawn)
	{
		SelectedMapPawn->RotateCameraPitch(AxisValue, DeltaTime);
	}
}

void AConnectedPlayer::RotatePawnCameraLeftRight(const float & AxisValue, const float & DeltaTime)
{
	if (SelectedMapPawn)
	{
		SelectedMapPawn->RotateCameraYaw(AxisValue, DeltaTime);
	}
}

void AConnectedPlayer::ZoomPawnCameraInOut(const float & AxisValue, const float & DeltaTime)
{
	if (SelectedMapPawn)
	{
		SelectedMapPawn->ZoomCamera(AxisValue, DeltaTime);
	}
}

CONNECTED_PLAYER_CAMERA AConnectedPlayer::GetCameraType() const
{
	return CameraType;
}

void AConnectedPlayer::OnSpectateReplicated()
{
	if (Role == ROLE_AutonomousProxy) 
	{
		for (TActorIterator<AMapPawn> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			if (SpectatingPawnID == ActorItr->GetPawnID())
			{
				SelectedMapPawn = (*ActorItr);
			}
		}
	}
}

void AConnectedPlayer::ServerBeginTurnBasedMechanics_Implementation()
{
	TActorIterator<AWorldGrid> WorldGrid(GetWorld());
	if (WorldGrid)
	{
		WorldGrid->BeginTurnBasedMechanics();
	}
	else
	{
		UE_LOG(LogNotice, Warning, TEXT("No Grid found while starting turn based mechanics"));
	}
}

bool AConnectedPlayer::ServerBeginTurnBasedMechanics_Validate()
{
	return true;
}

void AConnectedPlayer::ServerEndTurnBasedMechanics_Implementation()
{
	TActorIterator<AWorldGrid> WorldGrid(GetWorld());
	if (WorldGrid)
	{
		WorldGrid->EndTurnBasedMechanics();
	}
	else
	{
		UE_LOG(LogNotice, Warning, TEXT("No Grid found while ending turn based mechanics"));
	}
}

bool AConnectedPlayer::ServerEndTurnBasedMechanics_Validate()
{
	return true;
}

void AConnectedPlayer::ServerMovePlayer_Implementation(const FVector & Location, const int PawnID)
{
	TActorIterator<AWorldGrid> WorldGrid(GetWorld());
	if (WorldGrid)
	{
		WorldGrid->MoveCharacter(PawnID, Location, (WorldGrid->GetHostID() == GetPlayerID()) ? true : false);
	}
	else
	{
		UE_LOG(LogNotice, Warning, TEXT("No Grid found while moving player"));
	}
}

bool AConnectedPlayer::ServerMovePlayer_Validate(const FVector & Location, const int PawnID)
{
	return true;
}
