// Fill out your copyright Warning in the Description page of Project Settings.
#include "GM_Adventure_MainMenu.h"

#include "Grid/WorldGrid2D.h"
#include "Character/MapPawn.h"
#include "UObject/ConstructorHelpers.h"
#include "Adventure.h"

AGM_Adventure_MainMenu::AGM_Adventure_MainMenu()
{
	UE_LOG(LogNotice, Display, TEXT("Using GameMode:  %s"), *this->GetName());
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/Characters/MapPawn/BP_MapPawn"));
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerController(TEXT("/Script/Adventure.PC_Adventure_Default"));

	if (!PlayerPawnBPClass.Class)
	{
		UE_LOG(LogNotice, Error, TEXT("NO DEFAULT PLAYER PAWN CLASS FOUND"));
	}

	if (!PlayerController.Class)
	{
		UE_LOG(LogNotice, Error, TEXT("NO DEFAULT PLAYER CONTROLLER CLASS FOUND"));
	}

	DefaultPawnClass = PlayerPawnBPClass.Class;
	PlayerControllerClass = PlayerController.Class;
}

void AGM_Adventure_MainMenu::InitGame(const FString & MapName, const FString & Options, FString & ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	TActorIterator<AWorldGrid2D> GridItr(GetWorld());
	if (GridItr)
	{
		GridItr->MakeGrid(10, 10);

		//Make spawn points
		TArray<FGridCoordinate> SpawnLocations = {
			{0, 0},
			{0, 1},
			{0, 4},
			{0 ,6}
		};

		GridItr->SetSpawnLocations(SpawnLocations);

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

	TActorIterator<AWorldGrid2D> GridItr(GetWorld());
	if (GridItr)
	{
		FGridCoordinate SpawnLocation;
		if (GridItr->GetOpenSpawnLocation(SpawnLocation))
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
