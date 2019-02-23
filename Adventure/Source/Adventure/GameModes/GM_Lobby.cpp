// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.
#include "GM_Lobby.h"

#include "Adventure.h"
#include "GI_Adventure.h"
#include "Grid/WorldGrid.h"
#include "Widgets/HUD_MPLobby.h"
#include "PlayerStates/PS_Multiplayer.h"
#include "GameStates/GS_Multiplayer.h"
#include "DownloadManager/DownloadManager.h"

AGM_Lobby::AGM_Lobby()
{
	m_bMapHasBeenQueued = false;
	bUseSeamlessTravel = true;
	m_PlayerIndexCount = 0;
}

void AGM_Lobby::StartGame()
{
	TActorIterator<ADownloadManager> DLManager(GetWorld());
	if (DLManager)
	{
		DLManager->Subscribe();
	}
	else
	{
		UE_LOG(LogNotice, Warning, TEXT("<DownloadManager>: No valid download manager found in level."));
	}

	UGI_Adventure* GameInstance = Cast<UGI_Adventure>(GetGameInstance());
	auto GS = GetGameState<AGS_Multiplayer>();
	if (GameInstance &&  GS)
	{
		APS_Multiplayer* PS = Cast<APS_Multiplayer>(GS->PlayerArray[0]);
		if (PS)
		{
			if (PS->ServerOnly_LoadMap(m_CurrentMapName))
			{
				m_bMapHasBeenQueued = true;
				GS->ServerOnly_LoadMapOnClients(m_CurrentMapName);
			}
			else
			{
				PS->GenerateEmptyMap(m_CurrentMapName, FGridCoordinate(10, 10));
			}
		}

		GameInstance->StartSession();
	}
}

void AGM_Lobby::SetMapToLoad(const FString & Name)
{
	m_CurrentMapName = Name;
}

void AGM_Lobby::GetMapToLoad(FString & Name)const
{
	Name = m_CurrentMapName;
}

void AGM_Lobby::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	UE_LOG(LogNotice, Warning, TEXT("<HandleNewConnection>: New player joined!"));

	// Assign a unique ID for the game
	LoginConnectedPlayer(NewPlayer);

	if (m_bMapHasBeenQueued)
	{
		// Load the map if server or download the map if client
		APS_Multiplayer* PS = Cast<APS_Multiplayer>(NewPlayer->PlayerState);
		if (PS)
		{
			if (m_PlayerIndexCount > 1)
			{
				if (!PS->SetupNetworking())
				{
					PS->GenerateEmptyMap(m_CurrentMapName, FGridCoordinate(10, 10));
				}
			}
		}
	}
}

int AGM_Lobby::GeneratePlayerID()
{
	return m_PlayerIndexCount++;
}

void AGM_Lobby::LoginConnectedPlayer(AController * Player)
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
