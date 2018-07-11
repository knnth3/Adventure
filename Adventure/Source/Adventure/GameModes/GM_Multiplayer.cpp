// Fill out your copyright notice in the Description page of Project Settings.
#include "GM_Multiplayer.h"

#include "GI_Adventure.h"
#include "Adventure.h"
#include "Grid/WorldGrid.h"
#include "GameStates/GS_Multiplayer.h"


AGM_Multiplayer::AGM_Multiplayer()
{

	GridDimensions.X = 10;
	GridDimensions.Y = 10;

}

void AGM_Multiplayer::InitGame(const FString & MapName, const FString & Options, FString & ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	FVector Location(0.0f);
	FString MapFileName = UGameplayStatics::ParseOption(Options, "SN");


	//Make sure the object being spawned has collision turned off
	//The spawner's spawn param does not work and will fail if the object
	//is barely colliding.
	TActorIterator<APlayerStart> SpawnerItr(GetWorld());
	if (SpawnerItr)
	{
		SpawnerItr->GetCapsuleComponent()->SetMobility(EComponentMobility::Movable);
	}

	//UGI_Adventure* GameInstance = Cast<UGI_Adventure>(GetGameInstance());
	//if (GameInstance)
	//{
	//	FHOSTGAME_SETTINGS settings = GameInstance->GetHostSettings();
	//	MapFileName = settings.MapName;
	//}

	WorldGrid = Cast<AWorldGrid>(GetWorld()->SpawnActor(*GridClass, &Location));
	if (WorldGrid)
	{
		bool MapLoaded = OnLoadMapRequest(MapFileName);
		WorldGrid->Initialize(GridDimensions.X, GridDimensions.Y);

		if (MapLoaded && PendingObjects.Num() != 0)
		{
			for (const auto& object : PendingObjects)
			{
				switch (object.Type)
				{
				case GAMEBUILDER_OBJECT_TYPE::ANY:
					break;
				case GAMEBUILDER_OBJECT_TYPE::INTERACTABLE:
					RequestSpawnInteractible(object.ModelIndex, object.Location);
					break;
				case GAMEBUILDER_OBJECT_TYPE::SPAWN:
					RequestSetSpawnLocation(object.ModelIndex, object.Location);
					break;
				case GAMEBUILDER_OBJECT_TYPE::NPC:
					break;
				default:
					break;
				}
			}

			PendingObjects.Empty();
		}
		else
		{
			UE_LOG(LogNotice, Error, TEXT("Load map failed. Default Loaded. | Map Loaded:  %b | Objects found: %i"), MapLoaded, PendingObjects.Num());
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

bool AGM_Multiplayer::RequestSpawnInteractible(int Type, const FGridCoordinate & Location)
{
	bool Success = true;
	TActorIterator<AWorldGrid> GridItr(GetWorld());
	if (GridItr)
	{
		if (!GridItr->AddInteractable(Type, Location))
		{
			Success = false;
		}
	}
	return Success;
}

bool AGM_Multiplayer::RequestDeleteObject(GAMEBUILDER_OBJECT_TYPE Type, const FGridCoordinate & Location)
{
	bool Success = false;
	TActorIterator<AWorldGrid> GridItr(GetWorld());
	if (GridItr)
	{
		switch (Type)
		{
		case GAMEBUILDER_OBJECT_TYPE::INTERACTABLE:
			Success = GridItr->RemoveInteractable(Location);
			break;
		case GAMEBUILDER_OBJECT_TYPE::SPAWN:
			Success = GridItr->RemoveSpawnLocation(Location);
			break;
		default:
			break;
		}
	}
	return Success;
}

bool AGM_Multiplayer::RequestSetSpawnLocation(int Type, const FGridCoordinate & Location)
{
	bool Success = true;
	TActorIterator<AWorldGrid> GridItr(GetWorld());
	if (GridItr)
	{
		if (!GridItr->SetSpawnLocation(Type, Location))
		{
			Success = false;
		}
	}
	return Success;
}

void AGM_Multiplayer::SetGridDimensions(const FGridCoordinate & Dimensions)
{
	GridDimensions = Dimensions;
}

void AGM_Multiplayer::AddObjectForPreInit(const FGAMEBUILDER_OBJECT & object)
{
	PendingObjects.Push(object);
}

void AGM_Multiplayer::SetGridDimensions2(const FGridCoordinate & Dimensions)
{
}

FGridCoordinate AGM_Multiplayer::GetGridDimensions() const
{
	return GridDimensions;
}

bool AGM_Multiplayer::OnLoadMapRequest_Implementation(const FString& SaveSlot)
{
	return false;
}
