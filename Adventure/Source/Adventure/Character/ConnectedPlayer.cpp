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
	CameraType = CONNECTED_PLAYER_CAMERA::CHARACTER;
	SelectedPawn = nullptr;
	SpectatingPawnID = 0;

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
				WorldGrid->RegisterPlayerController(this);

				// Add two new characters
				for (int x = 0; x < 2; x++)
				{
					int NewPawnID = WorldGrid->AddCharacter(GetPlayerID());
					if (NewPawnID)
					{
						SpectatingPawnID = NewPawnID;
						OwningPawns.Push(NewPawnID);
						SelectedPawn = WorldGrid->GetPawn(GetPlayerID(), NewPawnID);
						Client_SetFocusToSelectedPawn();
					}
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
	DOREPLIFETIME(AConnectedPlayer, CurrentState);
	DOREPLIFETIME(AConnectedPlayer, OwningPawns);
}

void AConnectedPlayer::SetPlayerState(const TURN_BASED_STATE NewState)
{
	CurrentState = NewState;
	Client_NotifyStateChange();
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
	if (SelectedPawn)
	{
		Location = SelectedPawn->GetActorLocation();
		return true;
	}
	return false;
}

void AConnectedPlayer::Server_ScaleHead_Implementation(const FVector & Size)
{
	if (SelectedPawn)
	{
		SelectedPawn->ScaleHead(Size);
	}
}

bool AConnectedPlayer::Server_ScaleHead_Validate(const FVector & Size)
{
	return true;
}

void AConnectedPlayer::SetSpectatingPawn(const int Index)
{
	Server_SetSpectatingPawn(Index);
}

void AConnectedPlayer::BeginCombat(const TArray<int>& PlayerOrder)
{
	Server_BeginTurnBasedMechanics(PlayerOrder);
}

void AConnectedPlayer::EndCombat()
{
	Server_EndTurnBasedMechanics();
}

// Called when the game starts or when spawned
void AConnectedPlayer::BeginPlay()
{
	Super::BeginPlay();
}

int AConnectedPlayer::GetNumOwningPawns() const
{
	return OwningPawns.Num();
}

bool AConnectedPlayer::GetSelectedActorLocation(FVector& Location) const
{
	if (SelectedPawn)
	{
		Location = SelectedPawn->GetActorLocation();
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
	if (SelectedPawn)
	{
		Server_MovePlayer(Location, SpectatingPawnID);
	}
}

void AConnectedPlayer::SetPawnTargetLocation(const FVector & Location)
{
	if (SelectedPawn)
	{
		SelectedPawn->SetTarget(Location);
	}
}

void AConnectedPlayer::ClearPawnTargetLocation()
{
	if (SelectedPawn)
	{
		SelectedPawn->ClearTarget();
	}
}

void AConnectedPlayer::Attack(AMapPawnAttack* Attack, const FVector & EndLocation)
{
	Server_Attack(Attack, EndLocation);
}

void AConnectedPlayer::SetCameraToOverview(const float time)
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController)
	{
		PlayerController->SetViewTargetWithBlend(this, time, VTBlend_Linear, 0.0f, true);
		CameraType = CONNECTED_PLAYER_CAMERA::OVERVIEW;
	}
}

void AConnectedPlayer::SetCameraToCharacter(const float time)
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController)
	{
		PlayerController->SetViewTargetWithBlend(SelectedPawn, time, VTBlend_Linear, 0.0f, true);
		CameraType = CONNECTED_PLAYER_CAMERA::CHARACTER;
	}
}

void AConnectedPlayer::SwapCameraView(const float& time)
{
	if (SelectedPawn)
	{
		switch (CameraType)
		{
		case CONNECTED_PLAYER_CAMERA::CHARACTER:
			SetCameraToOverview(time);
			break;
		case CONNECTED_PLAYER_CAMERA::OVERVIEW:
			SetCameraToCharacter(time);
			break;
		case CONNECTED_PLAYER_CAMERA::NONE:
		default:
			UE_LOG(LogNotice, Warning, TEXT("Connected player camera type is invalid"));
			break;
		}
	}
	else
	{
		UE_LOG(LogNotice, Warning, TEXT("No Map pawn found while swapping cameras"));
	}
}

void AConnectedPlayer::RotatePawnCameraUpDown(const float & AxisValue, const float & DeltaTime)
{
	if (SelectedPawn)
	{
		SelectedPawn->RotateCameraPitch(AxisValue, DeltaTime);
	}
}

void AConnectedPlayer::RotatePawnCameraLeftRight(const float & AxisValue, const float & DeltaTime)
{
	if (SelectedPawn)
	{
		SelectedPawn->RotateCameraYaw(AxisValue, DeltaTime);
	}
}

void AConnectedPlayer::ZoomPawnCameraInOut(const float & AxisValue, const float & DeltaTime)
{
	if (SelectedPawn)
	{
		SelectedPawn->ZoomCamera(AxisValue, DeltaTime);
	}
}

CONNECTED_PLAYER_CAMERA AConnectedPlayer::GetCameraType() const
{
	return CameraType;
}

void AConnectedPlayer::Server_Attack_Implementation(AMapPawnAttack* Attack, const FVector & EndLocation)
{
	if (SelectedPawn)
	{
		TActorIterator<AWorldGrid> WorldGrid(GetWorld());
		if (WorldGrid->GetHostID() == GetPlayerID() || WorldGrid && WorldGrid->IsTurn(SpectatingPawnID))
		{
			SelectedPawn->Attack(Attack, EndLocation);
		}
	}
	else
	{
		UE_LOG(LogNotice, Warning, TEXT("Attack does not replicate over the network!"));
	}
}

bool AConnectedPlayer::Server_Attack_Validate(AMapPawnAttack* Attack, const FVector & EndLocation)
{
	return true;
}

void AConnectedPlayer::Server_BeginTurnBasedMechanics_Implementation(const TArray<int>& Order)
{
	TActorIterator<AWorldGrid> WorldGrid(GetWorld());
	if (WorldGrid && WorldGrid->GetHostID() == GetPlayerID())
	{
		WorldGrid->BeginTurnBasedMechanics(Order);
	}
	else
	{
		UE_LOG(LogNotice, Warning, TEXT("No Grid found while starting turn based mechanics"));
	}
}

bool AConnectedPlayer::Server_BeginTurnBasedMechanics_Validate(const TArray<int>& Order)
{
	return true;
}

void AConnectedPlayer::Server_EndTurnBasedMechanics_Implementation()
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

bool AConnectedPlayer::Server_EndTurnBasedMechanics_Validate()
{
	return true;
}

void AConnectedPlayer::Server_MovePlayer_Implementation(const FVector & Location, const int PawnID)
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

bool AConnectedPlayer::Server_MovePlayer_Validate(const FVector & Location, const int PawnID)
{
	return true;
}

void AConnectedPlayer::Server_SetSpectatingPawn_Implementation(const int PawnIndex)
{
	if (OwningPawns.Num() > PawnIndex && PawnIndex >= 0)
	{
		TActorIterator<AWorldGrid> WorldGrid(GetWorld());
		if (WorldGrid)
		{
			int NewPawnID = OwningPawns[PawnIndex];
			SpectatingPawnID = NewPawnID;
			SelectedPawn = WorldGrid->GetPawn(GetPlayerID(), NewPawnID);
			Client_SetFocusToSelectedPawn();
		}
	}
}

bool AConnectedPlayer::Server_SetSpectatingPawn_Validate(const int PawnIndex)
{
	return true;
}

void AConnectedPlayer::Client_SetFocusToSelectedPawn_Implementation()
{
	if (Role == ROLE_Authority)
	{
		for (TActorIterator<AMapPawn> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			if (SpectatingPawnID == ActorItr->GetPawnID())
			{
				SelectedPawn = (*ActorItr);

				if (CameraType == CONNECTED_PLAYER_CAMERA::CHARACTER)
				{
					SetCameraToCharacter(0.5f);
				}
			}
		}
	}
}

void AConnectedPlayer::Client_NotifyStateChange_Implementation()
{
	OnPlayerStatusChanged(CurrentState);
}
