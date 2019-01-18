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
	m_MapDNE = false;
	bUseSeamlessTravel = true;
	m_PlayerIndexCount = 0;
	m_GridDimensions = { 100, 100 };
}

FGridCoordinate AGM_Multiplayer::GetMapSize()const
{
	return m_GridDimensions;
}

FString AGM_Multiplayer::GetMapName() const
{
	return m_CurrentMapName;
}

int AGM_Multiplayer::GetHostID() const
{
	return m_ConnnectedPlayers.at(TCHAR_TO_UTF8(*m_HostUsername));
}

void AGM_Multiplayer::InitGame(const FString & MapName, const FString & Options, FString & ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	m_CurrentMapName = UGameplayStatics::ParseOption(Options, "SN");
}

void AGM_Multiplayer::HandleStartingNewPlayer_Implementation(APlayerController * NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	LoginConnectedPlayer(NewPlayer);
}

void AGM_Multiplayer::HandleSeamlessTravelPlayer(AController *& C)
{
	Super::HandleSeamlessTravelPlayer(C);

	UE_LOG(LogNotice, Warning, TEXT("<HandleNewConnection>: %s has joined via seamless travel."), *C->PlayerState->GetPlayerName());
	// LoginConnectedPlayer(C);
}

void AGM_Multiplayer::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();


	UE_LOG(LogNotice, Warning, TEXT("<HandleNewConnection>: All players from previous level have joined."));
}

void AGM_Multiplayer::PostLogin(APlayerController * NewPlayer)
{
	Super::PostLogin(NewPlayer);

	UE_LOG(LogNotice, Warning, TEXT("<HandleNewConnection>: Client is ready to recieve map download!"));
	APS_Multiplayer* PS = NewPlayer->GetPlayerState<APS_Multiplayer>();
	TActorIterator<AWorldGrid> WorldGrid(GetWorld());
	if (PS && WorldGrid && (m_MapDNE || !PS->LoadMap(WorldGrid->GetMapName())))
	{
		m_MapDNE = true;
		PS->GenerateEmptyMap(WorldGrid->GetMapName(), WorldGrid->GetMapSize());
	}
}

int AGM_Multiplayer::GeneratePlayerID()
{
	return m_PlayerIndexCount++;
}

// Ensure the function remains fast and simple or client will hang
void AGM_Multiplayer::LoginConnectedPlayer(AController * Player)
{
	APS_Multiplayer* currentPlayerState = Cast<APS_Multiplayer>(Player->PlayerState);
	AGS_Multiplayer* gameState = Cast<AGS_Multiplayer>(GameState);

	if (currentPlayerState && gameState)
	{
		std::string PlayerName = TCHAR_TO_UTF8(*currentPlayerState->GetPlayerName());

		//First to connect will be the owner
		if (m_ConnnectedPlayers.empty())
		{
			m_HostUsername = FString(PlayerName.c_str());
			UE_LOG(LogNotice, Warning, TEXT("<ServerSetup>: Host registered as %s"), *m_HostUsername);
		}

		// New player has joined
		if (m_ConnnectedPlayers.find(PlayerName) == m_ConnnectedPlayers.end())
		{
			m_ConnnectedPlayers[PlayerName] = GeneratePlayerID();
			currentPlayerState->ServerOnly_SetGameID(m_ConnnectedPlayers[PlayerName]);

			gameState->AddNewPlayer(m_ConnnectedPlayers[PlayerName], currentPlayerState->GetPlayerName());

			UE_LOG(LogNotice, Warning, TEXT("<HandleNewConnection>: %s has connected. Player was assigned to GameID: %i"), *FString(PlayerName.c_str()), currentPlayerState->GetGameID());
		}
		else
		{
			currentPlayerState->ServerOnly_SetGameID(m_ConnnectedPlayers[PlayerName]);
			UE_LOG(LogNotice, Warning, TEXT("<HandleNewConnection>: %s has reconnected."), *FString(PlayerName.c_str()));
		}
	}
}
