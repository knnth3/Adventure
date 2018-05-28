// Fill out your copyright notice in the Description page of Project Settings.
#include "GM_Multiplayer.h"

#include "GI_Adventure.h"
#include "MapFileReader/MapFileReader.h"
#include "GameStates/GS_Multiplayer.h"


AGM_Multiplayer::AGM_Multiplayer()
{
	GridRows = 5;
	GridColumns = 5;

	static ConstructorHelpers::FClassFinder<APawn> BP_PlayerPawn(TEXT("/Game/Blueprints/Characters/MapPawn/BP_MapPawn"));
	static ConstructorHelpers::FClassFinder<APlayerController> BP_PlayerController(TEXT("/Game/Blueprints/PlayerControllers/BP_PC_Adventure_Default"));
	static ConstructorHelpers::FClassFinder<AGameStateBase> BP_GameState(TEXT("/Game/Blueprints/GameStates/BP_GS_Multiplayer"));
	static ConstructorHelpers::FClassFinder<AWorldGrid> BP_WorldGrid(TEXT("/Game/Blueprints/Grid/BP_WorldGrid"));

	if (!BP_PlayerPawn.Class)
	{
		UE_LOG(LogNotice, Error, TEXT("NO DEFAULT PLAYER PAWN CLASS FOUND"));
	}

	if (!BP_PlayerController.Class)
	{
		UE_LOG(LogNotice, Error, TEXT("NO DEFAULT PLAYER CONTROLLER CLASS FOUND"));
	}

	if (!BP_GameState.Class)
	{
		UE_LOG(LogNotice, Error, TEXT("NO DEFAULT GAMESTATE CLASS FOUND"));
	}

	if (!BP_WorldGrid.Class)
	{
		UE_LOG(LogNotice, Error, TEXT("NO WORLD GRID CLASS FOUND"));
	}

	DefaultPawnClass = BP_PlayerPawn.Class;
	PlayerControllerClass = BP_PlayerController.Class;
	GameStateClass = BP_GameState.Class;
	GridClass = BP_WorldGrid.Class;
}

void AGM_Multiplayer::InitGame(const FString & MapName, const FString & Options, FString & ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	UGI_Adventure* GameInstance = Cast<UGI_Adventure>(GetGameInstance());

	if (GameInstance)
	{
		FHOSTGAME_SETTINGS settings = GameInstance->GetHostSettings();
		FString MapFileName = settings.MapName;

		FMapSettings Settings;

		//ReadFile
		if (MapFileReader::LoadMapFile(MapFileName, Settings))
		{
			//Map Loaded Successfully
			UE_LOG(LogNotice, Warning, TEXT("Map loaded Successfully"));

			int* rows = Settings.Get<int>("GridRows");
			if (rows)
			{
				GridRows = *rows;
			}

			int* columns = Settings.Get<int>("GridColumns");
			if (columns)
			{
				GridColumns = *columns;
			}
		}
	}

	//Make sure the object being spawned has collision turned off
	//The spawner's spawn param does not work and will fail if the object
	//is barely colliding.
	TActorIterator<APlayerStart> SpawnerItr(GetWorld());
	if (SpawnerItr)
	{
		SpawnerItr->GetCapsuleComponent()->SetMobility(EComponentMobility::Movable);
	}

	FVector Location(0.0f);
	WorldGrid = Cast<AWorldGrid>(GetWorld()->SpawnActor(*GridClass, &Location));

	if (WorldGrid)
	{

		WorldGrid->Initialize(GridRows, GridColumns);

		//Make spawn points
		TArray<FGridCoordinate> SpawnLocations = 
		{
		{ 0, 0 },
		{ 0, 1 },
		{ 0, 4 },
		{ 0 ,6 }
		};

		WorldGrid->SetSpawnLocations(SpawnLocations);
	}
	else
	{
		UE_LOG(LogNotice, Error, TEXT("NO WORLD GRID FOUND."));
	}
}

AActor * AGM_Multiplayer::ChoosePlayerStart_Implementation(AController * Player)
{
	if (WorldGrid)
	{
		FGridCoordinate SpawnLocation;
		if (WorldGrid->GetOpenSpawnLocation(SpawnLocation))
		{
			UE_LOG(LogNotice, Warning, TEXT("Spawn point checked out: (%d, %d)"), SpawnLocation.X, SpawnLocation.Y);

			/** This also works */
			//TArray<AActor*> PlayerStarts;
			//UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);

			TActorIterator<APlayerStart> SpawnerItr(GetWorld());
			if (SpawnerItr)
			{
				SpawnerItr->SetActorLocation(UGridFunctions::GridToWorldLocation(SpawnLocation));

				return *SpawnerItr;
			}
			else
			{
				UE_LOG(LogNotice, Error, TEXT("NO SPAWNER FOUND!"));
			}
		}
	}
	else
	{
		UE_LOG(LogNotice, Error, TEXT("COULD NOT SPAWN CHARACTER CORRECTLY"));
	}

	return Super::ChoosePlayerStart_Implementation(Player);
}

void AGM_Multiplayer::GetGridDimensions(int & Rows, int & Columns)const
{
	Rows = GridRows;
	Columns = GridColumns;
}
