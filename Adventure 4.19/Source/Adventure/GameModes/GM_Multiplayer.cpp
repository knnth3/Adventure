// Fill out your copyright notice in the Description page of Project Settings.
#include "GM_Multiplayer.h"

#include "GI_Adventure.h"
#include "Adventure.h"
#include "Grid/WorldGrid.h"
#include "Character/SpectatorMapPawn.h"
#include "GameStates/GS_Multiplayer.h"
#include "Character/ConnectedPlayer.h"


AGM_Multiplayer::AGM_Multiplayer()
{

	GridDimensions.X = 10;
	GridDimensions.Y = 10;
	PlayersConnected = 0;
	CurrentPlayersTurn = 0;
	CurrentState = GM_MULTIPLAYER_STATE::ROAMING;
}

void AGM_Multiplayer::SetGameState(GM_MULTIPLAYER_STATE State)
{
	CurrentState = State;
}

bool AGM_Multiplayer::IsPlayersTurn(const int PlayerID)
{
	return (CurrentPlayersTurn == PlayerID);
}

void AGM_Multiplayer::EndTurn(const int PlayerID)
{
	if (IsPlayersTurn(PlayerID))
	{
		//Get A new player ID
		UE_LOG(LogNotice, Warning, TEXT("Turn Ended for Player: %i"), PlayerID);
	}
	else
	{
		UE_LOG(LogNotice, Warning, TEXT("It's not your turn dummy :)"), PlayerID);
	}
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

			//Temp spawn actors to test interactability
			WorldGrid->SpawnMapPawn();
			WorldGrid->SpawnMapPawn();
			WorldGrid->SpawnMapPawn();
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

void AGM_Multiplayer::PostLogin(APlayerController * NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (NewPlayer->PlayerState)
	{
		AConnectedPlayer* Player = Cast<AConnectedPlayer>(NewPlayer->GetPawn());
		if (Player)
		{
			int32 PlayerID = NewPlayer->PlayerState->PlayerId;
			if (ConnectedPlayers.find(PlayerID) != ConnectedPlayers.end())
			{
				ConnectedPlayers[PlayerID] = Player;
				Player->SpectateMapPawn();
			}
			else if (WorldGrid)
			{ 
				if (WorldGrid->SpawnMapPawn(PlayerID))
				{
					ConnectedPlayers[NewPlayer->PlayerState->PlayerId] = Player;
					Player->SpectateMapPawn();
					UE_LOG(LogNotice, Error, TEXT("New Map Pawn Spawn Success!"));
				}
				else
				{
					UE_LOG(LogNotice, Error, TEXT("Failed to spawn new Map Pawn."));
				}
			}
			else
			{
				UE_LOG(LogNotice, Error, TEXT("No World Grid found while Login"));
			}
		}
		else
		{
			UE_LOG(LogNotice, Error, TEXT("Pawn failed to cast to AConnectedPlayer while Login"));
		}
	}
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

FGridCoordinate AGM_Multiplayer::GetGridDimensions() const
{
	return GridDimensions;
}

GM_MULTIPLAYER_STATE AGM_Multiplayer::GetGameState() const
{
	return CurrentState;
}

bool AGM_Multiplayer::OnLoadMapRequest_Implementation(const FString& SaveSlot)
{
	return false;
}
