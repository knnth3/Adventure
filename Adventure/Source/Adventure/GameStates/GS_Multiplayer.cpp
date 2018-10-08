// Fill out your copyright notice in the Description page of Project Settings.
#include "GS_Multiplayer.h"

#include "Grid/WorldGrid.h"
#include "Character/MapPawn.h"
#include "UObject/ConstructorHelpers.h"
#include "GameModes/GM_Multiplayer.h"
#include "PlayerStates/PS_Multiplayer.h"
#include "Adventure.h"

AGS_Multiplayer::AGS_Multiplayer()
{
	m_bFreeRoamActive = false;
	m_CurrentActivePlayer = 0;
}

void AGS_Multiplayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGS_Multiplayer, m_ActivePlayerName);
}

void AGS_Multiplayer::HandleBeginPlay()
{
	Super::HandleBeginPlay();

	if (HasAuthority())
	{
		AGM_Multiplayer* Gamemode = Cast<AGM_Multiplayer>(AuthorityGameMode);
		FGridCoordinate MapDimensions = Gamemode->GetMapSize();
		TActorIterator<AWorldGrid> WorldGridItr(GetWorld());
		if (WorldGridItr)
		{
			WorldGridItr->ServerOnly_GenerateGrid(MapDimensions);

			TArray<struct FGAMEBUILDER_OBJECT> Objects;
			Gamemode->GetMapObjects(Objects);

			for (auto& object : Objects)
			{
				switch (object.Type)
				{
				case GAMEBUILDER_OBJECT_TYPE::ANY:
					break;
				case GAMEBUILDER_OBJECT_TYPE::INTERACTABLE:
					WorldGridItr->ServerOnly_AddBlockingObject(object.ModelIndex, object.Location);
					break;
				case GAMEBUILDER_OBJECT_TYPE::SPAWN:
					WorldGridItr->ServerOnly_AddSpawnLocation(object.ModelIndex, object.Location);
					break;
				case GAMEBUILDER_OBJECT_TYPE::NPC:
					break;
				default:
					break;
				}
			}

			// Spawn actors
			FGridCoordinate SpawnLocation = WorldGridItr->ServerOnly_GetOpenSpawnLocation();
			WorldGridItr->ServerOnly_AddPawn(0, SpawnLocation, 0);
		}

		SetActivePlayer(-1);
	}
}

void AGS_Multiplayer::SetActivePlayer(const int ID)
{
	// Switch to someones turn
	if (ID != -1 && ID != m_CurrentActivePlayer)
	{
		UE_LOG(LogNotice, Warning, TEXT("Passing turn to: %i"), ID);
		m_bFreeRoamActive = false;
		m_CurrentActivePlayer = ID;

		for (auto& player : PlayerArray)
		{
			APS_Multiplayer* playerState = Cast<APS_Multiplayer>(player);
			if (playerState)
			{
				if (playerState->GetGameID() == ID)
				{
					m_ActivePlayerName = playerState->GetPlayerName();
					playerState->ServerOnly_SetPlayerState(TURN_BASED_STATE::ACTIVE);
				}
				else
				{
					playerState->ServerOnly_SetPlayerState(TURN_BASED_STATE::STANDBY);
				}
			}
		}
	}
	else if (!m_bFreeRoamActive && ID == -1)
	{
		UE_LOG(LogNotice, Warning, TEXT("Starting Free-Roam"));
		m_ActivePlayerName = TEXT("Free-Roam");
		m_bFreeRoamActive = true;
		m_CurrentActivePlayer = -1;

		for (auto& player : PlayerArray)
		{
			APS_Multiplayer* playerState = Cast<APS_Multiplayer>(player);
			if (playerState)
			{
				playerState->ServerOnly_SetPlayerState(TURN_BASED_STATE::FREE_ROAM);
			}
		}
	}
}

FString AGS_Multiplayer::GetActivePlayerName() const
{
	return m_ActivePlayerName;
}
