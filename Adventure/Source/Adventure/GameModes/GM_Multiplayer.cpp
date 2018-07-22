// Fill out your copyright notice in the Description page of Project Settings.
#include "GM_Multiplayer.h"

#include "GI_Adventure.h"
#include "Adventure.h"
#include "Grid/WorldGrid.h"
#include "Character/SpectatorMapPawn.h"
#include "GameStates/GS_Multiplayer.h"
#include "Character/ConnectedPlayer.h"
#include "Saves/MapSaveFile.h"
#include "Widgets/W_GameBuilderUI.h"

FGridCoordinate AGM_Multiplayer::GetMapSize()const
{
	return FGridCoordinate(Rows, Columns);
}

FString AGM_Multiplayer::GetMapName() const
{
	return MapName;
}

void AGM_Multiplayer::InitGame(const FString & MapName, const FString & Options, FString & ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	this->Rows = 10;
	this->Columns = 10;

	FVector Location(0.0f);
	FString MapFileName = UGameplayStatics::ParseOption(Options, "SN");
	UMapSaveFile* MapSaveFile = Cast<UMapSaveFile>(UGameplayStatics::LoadGameFromSlot(MapFileName, 0));
	if (MapSaveFile)
	{
		UE_LOG(LogNotice, Warning, TEXT("Map Loaded!"));
		UE_LOG(LogNotice, Warning, TEXT("Name: %s"), *MapSaveFile->MapName);
		UE_LOG(LogNotice, Warning, TEXT("Size: (%i, %i)"), MapSaveFile->MapSize.X, MapSaveFile->MapSize.Y);
		UE_LOG(LogNotice, Warning, TEXT("Number of Objects: %i"), MapSaveFile->Objects.Num());

		this->Rows = MapSaveFile->MapSize.X;
		this->Columns = MapSaveFile->MapSize.Y;
		PendingObjects = MapSaveFile->Objects;
	}

}

void AGM_Multiplayer::InitGameState()
{
	Super::InitGameState();

	AGS_Multiplayer* GameState = GetGameState<AGS_Multiplayer>();
	if (GameState)
	{
		GameState->Initialize(MapName, Rows, Columns);
	}
}

void AGM_Multiplayer::PostLogin(APlayerController * NewPlayer)
{
	Super::PostLogin(NewPlayer);

	TActorIterator<AWorldGrid> GridItr(GetWorld());
	if (GridItr)
	{
		int PlayerID = NewPlayer->PlayerState->PlayerId;
		UE_LOG(LogNotice, Log, TEXT("%s connected! ID= %i"), *NewPlayer->PlayerState->GetPlayerName(), PlayerID);

		if (!GridItr->AddCharacter(PlayerID))
		{
			UE_LOG(LogNotice, Warning, TEXT("No Spawns available to create character."));
		}
	}
	else
	{
		UE_LOG(LogNotice, Warning, TEXT("Player connected but character could not be spawned"));
	}
}

void AGM_Multiplayer::StartPlay()
{
	Super::StartPlay();

	TActorIterator<AWorldGrid> GridItr(GetWorld());
	if (GridItr)
	{
		for (const auto& object : PendingObjects)
		{
			switch (object.Type)
			{
			case GAMEBUILDER_OBJECT_TYPE::ANY:
				break;
			case GAMEBUILDER_OBJECT_TYPE::INTERACTABLE:
				GridItr->AddVisual(object.ModelIndex, object.Location);
				break;
			case GAMEBUILDER_OBJECT_TYPE::SPAWN:
				GridItr->AddSpawnLocation(object.ModelIndex, object.Location);
				break;
			case GAMEBUILDER_OBJECT_TYPE::NPC:
				break;
			default:
				break;
			}
		}

		GridItr->AddCharacter();
	}

	PendingObjects.Empty();
}