// Fill out your copyright Warning in the Description page of Project Settings.
#include "GM_Adventure_MainMenu.h"

#include "Grid/WorldGrid.h"
#include "Character/MapPawn.h"
#include "UObject/ConstructorHelpers.h"
#include "Adventure.h"

AGM_Adventure_MainMenu::AGM_Adventure_MainMenu()
{
	UE_LOG(LogNotice, Display, TEXT("Using GameMode:  %s"), *this->GetName());
	static ConstructorHelpers::FClassFinder<AWorldGrid> WorldGridBPClass(TEXT("/Game/Blueprints/Grid/BP_WorldGrid"));
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/Characters/MapPawn/BP_MapPawn"));
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerController(TEXT("/Game/Blueprints/PlayerControllers/BP_PC_Adventure_Default"));

	if (!PlayerPawnBPClass.Class)
	{
		UE_LOG(LogNotice, Error, TEXT("NO DEFAULT PLAYER PAWN CLASS FOUND"));
	}

	if (!PlayerController.Class)
	{
		UE_LOG(LogNotice, Error, TEXT("NO DEFAULT PLAYER CONTROLLER CLASS FOUND"));
	}

	if (!WorldGridBPClass.Class)
	{
		UE_LOG(LogNotice, Error, TEXT("NO BP WORLD GRID CLASS FOUND"));
	}

	GridClass = WorldGridBPClass.Class;
	DefaultPawnClass = PlayerPawnBPClass.Class;
	PlayerControllerClass = PlayerController.Class;
}

void AGM_Adventure_MainMenu::InitGame(const FString & MapName, const FString & Options, FString & ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	FVector Location(0.0f);
	WorldGrid = Cast<AWorldGrid>(GetWorld()->SpawnActor(*GridClass, &Location));

	if (WorldGrid)
	{
		WorldGrid->Initialize(5, 10);

		//Make spawn points
		TArray<FGridCoordinate> SpawnLocations = {
			{0, 0},
			{0, 1},
			{0, 4},
			{0 ,6}
		};

		WorldGrid->SetSpawnLocations(SpawnLocations);

		//Make sure the object being spawned has collision turned off
		//The spawner's spawn param does not work and will fail if the object
		//is barely colliding.
		TActorIterator<APlayerStart> SpawnerItr(GetWorld());
		if (SpawnerItr)
		{
			SpawnerItr->GetCapsuleComponent()->SetMobility(EComponentMobility::Movable);
		}
	}
	else
	{
		UE_LOG(LogNotice, Error, TEXT("NO WORLD GRID FOUND."));
	}
}

AActor* AGM_Adventure_MainMenu::ChoosePlayerStart_Implementation(AController* Player)
{
	UE_LOG(LogNotice, Warning, TEXT("New Character Spawned"));

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
