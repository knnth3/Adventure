// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.
#include "ConnectedPlayer.h"

#include "../PlayerControllers/PC_Multiplayer.h"
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
	SelectedPawn = nullptr;
	SpectatingPawnID = 0;
	UniqueID = -1;
	CameraTransitionAcceleration = 5500.0f;
	bRegistered = false;

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

	//Only run on server
	if (HasAuthority() && !bRegistered)
	{
		TActorIterator<AWorldGrid> WorldGridItr(GetWorld());
		if (WorldGridItr)
		{
			WorldGridItr->RegisterPlayerController(this);
			bRegistered = true;
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
	DOREPLIFETIME(AConnectedPlayer, UniqueID);
}

void AConnectedPlayer::SetPlayerState(const TURN_BASED_STATE NewState)
{
	CurrentState = NewState;
	Client_NotifyStateChange();
}

void AConnectedPlayer::AddNewCharacter(const int PawnID, bool IgnoreCameraLogic)
{
	OwningPawns.Push(PawnID);

	TActorIterator<AWorldGrid> WorldGridItr(GetWorld());
	if (WorldGridItr)
	{
		if (!SelectedPawn)
		{
			SpectatingPawnID = PawnID;
			SelectedPawn = WorldGridItr->GetPawn(GetPlayerID(), PawnID);
		}

		if (!IgnoreCameraLogic && CameraType == CONNECTED_PLAYER_CAMERA::CHARACTER)
		{
			Client_SetFocusToSelectedPawn();
		}
	}
}

void AConnectedPlayer::AdjustCameraToMap(const FGridCoordinate gridDimensions)
{
	FVector CStartLocation = UGridFunctions::GridToWorldLocation(UGridFunctions::WorldToGridLocation(FVector()));
	FVector CEndLocation = UGridFunctions::GridToWorldLocation(gridDimensions);

	FVector Center((CStartLocation.X + CEndLocation.X)*0.5f, (CStartLocation.Y + CEndLocation.Y)*0.5f, 0.0f);

	UE_LOG(LogNotice, Warning, TEXT("CameraLocation: %s"), *GetActorLocation().ToString());

	UE_LOG(LogNotice, Warning, TEXT("EndLocation GridSpace: %i, %i"), gridDimensions.X, gridDimensions.Y);
	UE_LOG(LogNotice, Warning, TEXT("EndLocation: %s"), *CEndLocation.ToString());
	UE_LOG(LogNotice, Warning, TEXT("Next CameraLocation: %s"), *Center.ToString());
	SetActorLocation(Center);
}

void AConnectedPlayer::SetPlayerID(int newID)
{
	UniqueID = newID;
}

int AConnectedPlayer::GetPlayerID() const
{
	return UniqueID;
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

void AConnectedPlayer::AddCharacter(int PlayerID, int PawnTypeIndex, bool OverrideSpawner, FVector NewLocation)
{
	Server_AddCharacter(PlayerID, PawnTypeIndex, OverrideSpawner, NewLocation);
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

bool AConnectedPlayer::GetSelectedActorStats(FMapPawnStatSheet & Stats) const
{
	if (SelectedPawn)
	{
		Stats = SelectedPawn->GetStatSheet();
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

void AConnectedPlayer::SetCameraToOverview()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController)
	{
		// Get Distance between the two cameras
		FVector StartLocation = SelectedPawn->GetCameraLocation();
		FVector EndLocation = FollowCamera->GetComponentLocation();
		float Distance = FVector::Distance(StartLocation, EndLocation);
		
		// Calculate Time
		float Time = FMath::Sqrt(2 * Distance / CameraTransitionAcceleration);

		PlayerController->SetViewTargetWithBlend(this, Time, VTBlend_Linear, 0.0f, true);
		CameraType = CONNECTED_PLAYER_CAMERA::OVERVIEW;
	}
}

void AConnectedPlayer::SetCameraToCharacter()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController)
	{
		// Get Distance between the two cameras
		FVector StartLocation = FollowCamera->GetComponentLocation();
		FVector EndLocation = SelectedPawn->GetCameraLocation();
		float Distance = FVector::Distance(StartLocation, EndLocation);
		
		// Calculate Time
		float Time = FMath::Sqrt(2 * Distance / CameraTransitionAcceleration);

		PlayerController->SetViewTargetWithBlend(SelectedPawn, Time, VTBlend_Linear, 0.0f, true);
		CameraType = CONNECTED_PLAYER_CAMERA::CHARACTER;
	}
}

void AConnectedPlayer::SwapCameraView()
{
	if (SelectedPawn)
	{
		switch (CameraType)
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
		TActorIterator<AWorldGrid> WorldGridItr(GetWorld());
		if (WorldGridItr)
		{
			if (WorldGridItr->GetHostID() == GetPlayerID() || WorldGridItr->IsTurn(SpectatingPawnID))
			{
				SelectedPawn->Attack(Attack, EndLocation);
			}
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
	TActorIterator<AWorldGrid> WorldGridItr(GetWorld());
	if (WorldGridItr && WorldGridItr->GetHostID() == GetPlayerID())
	{
		WorldGridItr->BeginTurnBasedMechanics(Order);
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
	TActorIterator<AWorldGrid> WorldGridItr(GetWorld());
	if (WorldGridItr)
	{
		WorldGridItr->EndTurnBasedMechanics();
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
	TActorIterator<AWorldGrid> WorldGridItr(GetWorld());
	if (WorldGridItr)
	{
		WorldGridItr->MoveCharacter(PawnID, Location, (WorldGridItr->GetHostID() == GetPlayerID()) ? true : false);
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

void AConnectedPlayer::Server_SetSpectatingPawn_Implementation(const int PawnIndex, bool focusPawn)
{
	if (OwningPawns.Num() > PawnIndex && PawnIndex >= 0)
	{
		TActorIterator<AWorldGrid> WorldGridItr(GetWorld());
		if (WorldGridItr)
		{
			int NewPawnID = OwningPawns[PawnIndex];
			SpectatingPawnID = NewPawnID;
			SelectedPawn = WorldGridItr->GetPawn(GetPlayerID(), NewPawnID);
			Client_SetFocusToSelectedPawn(focusPawn);
		}
	}
}

bool AConnectedPlayer::Server_SetSpectatingPawn_Validate(const int PawnIndex, bool focusPawn)
{
	return true;
}

void AConnectedPlayer::Client_SetFocusToSelectedPawn_Implementation(bool focusPawn)
{
	for (TActorIterator<AMapPawn> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		UE_LOG(LogNotice, Warning, TEXT("Found Pawn id=%i"), ActorItr->GetPawnID());
		if (SpectatingPawnID == ActorItr->GetPawnID())
		{
			UE_LOG(LogNotice, Warning, TEXT("Setting view to actor: %i"), SpectatingPawnID);
			SelectedPawn = (*ActorItr);
		}
	}
}

void AConnectedPlayer::Client_NotifyStateChange_Implementation()
{
	OnPlayerStatusChanged(CurrentState);
}

void AConnectedPlayer::Server_AddCharacter_Implementation(int PlayerID, int PawnTypeIndex, bool OverrideSpawner, FVector NewLocation)
{
	PlayerID = (PlayerID) ? PlayerID : GetPlayerID();
	TActorIterator<AWorldGrid> WorldGridItr(GetWorld());

	if (!WorldGridItr)
	{
		UE_LOG(LogNotice, Warning, TEXT("No Grid found while attempting to spawn new character."));
	}

	if (WorldGridItr->GetHostID() != GetPlayerID())
	{
		UE_LOG(LogNotice, Warning, TEXT("Unauthorized access to create character!: PlayerID=%i"), GetPlayerID());
		return;
	}

	WorldGridItr->AddCharacter(PlayerID, OverrideSpawner, NewLocation, PawnTypeIndex);
}

bool AConnectedPlayer::Server_AddCharacter_Validate(int PlayerID, int PawnTypeIndex, bool OverrideSpawner, FVector NewLocation)
{
	return true;
}

void AConnectedPlayer::Server_RegisterPlayer_Implementation() 
{
	TActorIterator<AWorldGrid> WorldGridItr(GetWorld());
	if (WorldGridItr)
	{
		int NewPawnID = WorldGridItr->AddCharacter(GetPlayerID(), false, FVector(), 0);
		SpectatingPawnID = NewPawnID;
		SelectedPawn = WorldGridItr->GetPawn(GetPlayerID(), NewPawnID);
		Client_SetFocusToSelectedPawn(true);
	}
}

bool AConnectedPlayer::Server_RegisterPlayer_Validate()
{
	return true;
}
