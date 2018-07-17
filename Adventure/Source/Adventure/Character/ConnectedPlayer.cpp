// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.
#include "ConnectedPlayer.h"

#include "Grid/WorldGrid.h"
#include "MapPawn.h"
#include "Adventure.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AConnectedPlayer::AConnectedPlayer()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PlayerType = CONNECTED_PLAYER_TYPE::NONE;
	CameraType = CONNECTED_PLAYER_CAMERA::OVERVIEW;
	CurrentMapPawn = nullptr;
	MapPawnID = -1;

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
	
}

void AConnectedPlayer::MovePlayer(const FVector & Location)
{
	UE_LOG(LogNotice, Warning, TEXT("Moving player %i"), MapPawnID);
	Server_MovePlayer(MapPawnID, Location);
}

void AConnectedPlayer::SwapCameraView(const float& time)
{
	if (CurrentMapPawn)
	{
		APlayerController* OurPlayerController = UGameplayStatics::GetPlayerController(this, 0);
		if (OurPlayerController)
		{
			switch (CameraType)
			{
			case CONNECTED_PLAYER_CAMERA::NONE:
				break;
			case CONNECTED_PLAYER_CAMERA::CHARACTER:
				OurPlayerController->SetViewTargetWithBlend(this, time);
				CameraType = CONNECTED_PLAYER_CAMERA::OVERVIEW;
				break;
			case CONNECTED_PLAYER_CAMERA::OVERVIEW:
				OurPlayerController->SetViewTargetWithBlend(CurrentMapPawn, time);
				CameraType = CONNECTED_PLAYER_CAMERA::CHARACTER;
				break;
			default:
				break;
			}
		}
	}
}

void AConnectedPlayer::RotatePawnCameraUpDown(const float & AxisValue, const float & DeltaTime)
{
	if (CurrentMapPawn)
	{
		CurrentMapPawn->RotateUpDown(AxisValue, DeltaTime);
	}
}

void AConnectedPlayer::RotatePawnCameraLeftRight(const float & AxisValue, const float & DeltaTime)
{
	if (CurrentMapPawn)
	{
		CurrentMapPawn->RotateLeftRight(AxisValue, DeltaTime);
	}
}

void AConnectedPlayer::ZoomPawnCameraInOut(const float & AxisValue, const float & DeltaTime)
{
	if (CurrentMapPawn)
	{
		CurrentMapPawn->ZoomInOut(AxisValue, DeltaTime);
	}
}

CONNECTED_PLAYER_CAMERA AConnectedPlayer::GetCameraType() const
{
	return CameraType;
}

void AConnectedPlayer::Server_MovePlayer_Implementation(int ID, const FVector & Location)
{
	if (CurrentMapPawn)
	{
		FGridCoordinate GridLocation = UGridFunctions::WorldToGridLocation(Location);
		CurrentMapPawn->SetDestination(GridLocation);
	}
}

bool AConnectedPlayer::Server_MovePlayer_Validate(int ID, const FVector & Location)
{
	return true;
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

int AConnectedPlayer::GetMapPawnID() const
{
	return MapPawnID;
}

void AConnectedPlayer::SetMapPawnID(const int ID)
{
	if (ID == -1)
	{
		MapPawnID = ID;
		CurrentMapPawn = nullptr;
		return;
	}

	TActorIterator<AWorldGrid> GridItr(GetWorld());
	if (GridItr)
	{
		AMapPawn* Pawn = GridItr->GetMapPawn(ID);
		if (Pawn)
		{
			MapPawnID = ID;
			CurrentMapPawn = Pawn;
		}
	}
}

//sets variables for replicaton over a network
void AConnectedPlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AConnectedPlayer, PlayerType);
	DOREPLIFETIME(AConnectedPlayer, MapPawnID);
}

