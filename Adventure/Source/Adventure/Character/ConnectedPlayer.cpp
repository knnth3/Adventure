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
	SpectateMapPawnID = 0;
	bSpectateNewPawn = true;

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

// Called when the game starts or when spawned
void AConnectedPlayer::BeginPlay()
{
	Super::BeginPlay();
	bSpectateNewPawn = true;
}

void AConnectedPlayer::MovePlayer(const FVector & Location, const int PawnID)
{
	Server_MovePlayer(PlayerState->PlayerId, Location, PawnID);
}

void AConnectedPlayer::SwapCameraView(const float& time)
{
	AMapPawn* MapPawn = GetMapPawn();
	if (MapPawn)
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
				PlayerController->SetViewTargetWithBlend(MapPawn, time, VTBlend_Linear, 0.0f, true);
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
	AMapPawn* MapPawn = GetMapPawn();
	if (MapPawn)
	{
		MapPawn->RotateUpDown(AxisValue, DeltaTime);
	}
}

void AConnectedPlayer::RotatePawnCameraLeftRight(const float & AxisValue, const float & DeltaTime)
{
	AMapPawn* MapPawn = GetMapPawn();
	if (MapPawn)
	{
		MapPawn->RotateLeftRight(AxisValue, DeltaTime);
	}
}

void AConnectedPlayer::ZoomPawnCameraInOut(const float & AxisValue, const float & DeltaTime)
{
	AMapPawn* MapPawn = GetMapPawn();
	if (MapPawn)
	{
		MapPawn->ZoomInOut(AxisValue, DeltaTime);
	}
}

CONNECTED_PLAYER_CAMERA AConnectedPlayer::GetCameraType() const
{
	return CameraType;
}

void AConnectedPlayer::Server_EndTurn_Implementation()
{
	UWorld* World = GetWorld();
	if (World)
	{
		AGM_Multiplayer* Gamemode = Cast<AGM_Multiplayer>(World->GetAuthGameMode());
		if (Gamemode)
		{
			//Gamemode->EndTurn(PlayerState->PlayerId);
		}
	}
}

bool AConnectedPlayer::Server_EndTurn_Validate()
{
	return true;
}

void AConnectedPlayer::Server_MovePlayer_Implementation(const int PlayerID, const FVector & Location, const int PawnID)
{
	TActorIterator<AWorldGrid> WorldGrid(GetWorld());
	if (WorldGrid)
	{
		WorldGrid->MoveCharacter(this, Location, PawnID);
	}
	else
	{
		UE_LOG(LogNotice, Warning, TEXT("No Grid found while moving player"));
	}
}

bool AConnectedPlayer::Server_MovePlayer_Validate(const int PlayerID, const FVector & Location, const int PawnID)
{
	return true;
}

// Called every frame
void AConnectedPlayer::Tick(float DeltaTime)
{
	static bool bRegistered = false;
	Super::Tick(DeltaTime);

	if (!bRegistered)
	{
		if (HasAuthority())
		{
			bRegistered = true;
			TActorIterator<AWorldGrid> WorldGrid(GetWorld());
			if (WorldGrid)
			{
				int CharacterID = 0;
				if (WorldGrid->RegisterPlayerController(this, CharacterID))
				{
					UE_LOG(LogNotice, Warning, TEXT("Player has been registered to the WorldGrid! PawnID = %i"), CharacterID);
				}
				else
				{
					UE_LOG(LogNotice, Error, TEXT("Failed to register player to the World Grid."));
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

void AConnectedPlayer::SpectateMapPawn(const int ID)
{
	SpectateMapPawnID = ID;
}

void AConnectedPlayer::OnNewSpectateFocus()
{
	bSpectateNewPawn = true;
}

AMapPawn * AConnectedPlayer::GetMapPawn()
{
	if (bSpectateNewPawn)
	{
		if (SpectateMapPawnID == -1)
		{
			for (TActorIterator<AMapPawn> ActorItr(GetWorld()); ActorItr; ++ActorItr)
			{
				UE_LOG(LogNotice, Warning, TEXT("Client: Map Pawn found: ID = '%i'. Looking for %i"), ActorItr->GetOwnerID(), SpectateMapPawnID);
				if (ActorItr->GetOwnerID() == PlayerState->PlayerId)
				{
					bSpectateNewPawn = false;
					SelectedMapPawn = *ActorItr;
					return SelectedMapPawn;
				}
			}
		}
		else if (HasAuthority())
		{
			for (TActorIterator<AMapPawn> ActorItr(GetWorld()); ActorItr; ++ActorItr)
			{
				UE_LOG(LogNotice, Warning, TEXT("Server: Map Pawn found: ID = '%i'. Looking for %i"), ActorItr->GetOwnerID(), SpectateMapPawnID);
				if (ActorItr->GetPawnID() == SpectateMapPawnID)
				{
					bSpectateNewPawn = false;
					SelectedMapPawn = *ActorItr;
					return SelectedMapPawn;
				}
			}
		}
	}
	else
	{
		return SelectedMapPawn;
	}

	UE_LOG(LogNotice, Warning, TEXT("Failed to get Map Pawn."));
	return nullptr;
}

//sets variables for replicaton over a network
void AConnectedPlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AConnectedPlayer, PlayerType);
	DOREPLIFETIME(AConnectedPlayer, SpectateMapPawnID);
}

