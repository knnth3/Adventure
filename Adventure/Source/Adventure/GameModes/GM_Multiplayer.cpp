// Fill out your copyright notice in the Description page of Project Settings.
#include "GM_Multiplayer.h"

#include "../PlayerControllers/PC_Multiplayer.h"
#include "PlayerStates/PS_Multiplayer.h"
#include "GI_Adventure.h"
#include "Adventure.h"
#include "Grid/WorldGrid.h"
#include "Character/SpectatorMapPawn.h"
#include "GameStates/GS_Multiplayer.h"
#include "Character/ConnectedPlayer.h"
#include "Saves/MapSaveFile.h"
#include "Widgets/W_GameBuilderUI.h"

AGM_Multiplayer::AGM_Multiplayer()
{
	m_PlayerIndexCount = 0;
}

FGridCoordinate AGM_Multiplayer::GetMapSize()const
{
	return m_GridDimensions;
}

FString AGM_Multiplayer::GetMapName() const
{
	return m_CurrentMapName;
}

void AGM_Multiplayer::GetMapObjects(TArray<struct FGAMEBUILDER_OBJECT>& Objects) const
{
	Objects = m_MapDecorations;
}

int AGM_Multiplayer::GetHostID() const
{
	return m_ConnnectedPlayers.at(m_HostUsername);
}

void AGM_Multiplayer::InitGame(const FString & MapName, const FString & Options, FString & ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	m_GridDimensions.X = 10;
	m_GridDimensions.Y = 10;
	FVector Location(0.0f);
	FString MapFileName = UGameplayStatics::ParseOption(Options, "SN");
	UMapSaveFile* MapSaveFile = Cast<UMapSaveFile>(UGameplayStatics::LoadGameFromSlot(MapFileName, 0));
	if (MapSaveFile)
	{
		UE_LOG(LogNotice, Warning, TEXT("Map Loaded!"));
		UE_LOG(LogNotice, Warning, TEXT("Name: %s"), *MapSaveFile->MapName);
		UE_LOG(LogNotice, Warning, TEXT("Size: (%i, %i)"), MapSaveFile->MapSize.X, MapSaveFile->MapSize.Y);
		UE_LOG(LogNotice, Warning, TEXT("Number of Objects: %i"), MapSaveFile->Objects.Num());

		m_GridDimensions.X = MapSaveFile->MapSize.X;
		m_GridDimensions.Y = MapSaveFile->MapSize.Y;
		m_MapDecorations = MapSaveFile->Objects;
	}
}

void AGM_Multiplayer::HandleStartingNewPlayer_Implementation(APlayerController * NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	APS_Multiplayer* currentPlayerState = Cast<APS_Multiplayer>(NewPlayer->PlayerState);
	if (currentPlayerState)
	{
		std::string PlayerName = TCHAR_TO_UTF8(*currentPlayerState->GetPlayerName());

		//Host should always connect first
		if (m_HostUsername.empty())
		{
			m_HostUsername = PlayerName;
			UE_LOG(LogNotice, Warning, TEXT("<ServerSetup>: Host registered as %s"), *FString(m_HostUsername.c_str()));
		}

		// New player has joined
		if (m_ConnnectedPlayers.find(PlayerName) == m_ConnnectedPlayers.end())
		{
			m_ConnnectedPlayers[PlayerName] = GeneratePlayerID();
			currentPlayerState->ServerOnly_SetGameID(m_ConnnectedPlayers[PlayerName]);

			UE_LOG(LogNotice, Warning, TEXT("<HandleNewConnection>: %s registered to id: %i"), *FString(PlayerName.c_str()), currentPlayerState->GetGameID());
		}
		else
		{
			currentPlayerState->ServerOnly_SetGameID(m_ConnnectedPlayers[PlayerName]);
			UE_LOG(LogNotice, Warning, TEXT("<HandleNewConnection>: %s has reconnected."), *FString(PlayerName.c_str()));
		}
	}
}

int AGM_Multiplayer::GeneratePlayerID()
{
	return m_PlayerIndexCount++;
}
