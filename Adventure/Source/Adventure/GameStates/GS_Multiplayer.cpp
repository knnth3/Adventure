// Fill out your copyright notice in the Description page of Project Settings.
#include "GS_Multiplayer.h"

#include "Grid/WorldGrid.h"
#include "Character/MapPawn.h"
#include "UObject/ConstructorHelpers.h"
#include "GameModes/GM_Multiplayer.h"
#include "Adventure.h"



void AGS_Multiplayer::HandleBeginPlay()
{
	Super::HandleBeginPlay();

	if (HasAuthority())
	{
		FVector Location(0.0f);
		AGM_Multiplayer* Gamemode = Cast<AGM_Multiplayer>(AuthorityGameMode);
		FGridCoordinate MapDimensions = Gamemode->GetMapSize();
		int HostID = Gamemode->GetHostID();

		WorldGrid = Cast<AWorldGrid>(GetWorld()->SpawnActor(*GridClass, &Location));
		if (WorldGrid)
		{
			//Add Map Decorations
			WorldGrid->Initialize(HostID, MapDimensions);
			TArray<struct FGAMEBUILDER_OBJECT> MapDecorations;
			if (Gamemode)
			{
				Gamemode->GetMapObjects(MapDecorations);
			}
			for (const auto& object : MapDecorations)
			{
				switch (object.Type)
				{
				case GAMEBUILDER_OBJECT_TYPE::ANY:
					break;
				case GAMEBUILDER_OBJECT_TYPE::INTERACTABLE:
					WorldGrid->AddVisual(object.ModelIndex, object.Location);
					break;
				case GAMEBUILDER_OBJECT_TYPE::SPAWN:
					WorldGrid->AddSpawnLocation(object.ModelIndex, object.Location);
					break;
				case GAMEBUILDER_OBJECT_TYPE::NPC:
					break;
				default:
					break;
				}
			}

			//Add map characters
			for (const auto& player : PlayerArray)
			{
				bool success = false;
				int PlayerID = player->PlayerId;
				TActorIterator<AWorldGrid> GridItr(GetWorld());
				if (GridItr)
				{
					GridItr->AddCharacter();
					if (/*HostID != PlayerID && */ !GridItr->AddCharacter(PlayerID))
					{
						UE_LOG(LogNotice, Error, TEXT("No Spawns available to create character."));
					}
					else
					{
						success = true;
					}
				}
				else
				{
					UE_LOG(LogNotice, Error, TEXT("Player connected but character could not be spawned"));
				}
				if (success)
				{
					UE_LOG(LogNotice, Log, TEXT("<HandleBeginPlay>: Created Pawn for: %s, id = %i"), *player->GetPlayerName(), PlayerID);
				}
			}
			return;
		}
	}
}