// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.
#include "GM_Multiplayer.h"

#include "Adventure.h"
#include "GI_Adventure.h"
#include "Grid/WorldGrid.h"
#include "Widgets/HUD_MPLobby.h"
#include "PlayerStates/PS_Multiplayer.h"
#include "GameStates/GS_Multiplayer.h"
#include "PlayerControllers/PC_Multiplayer.h"
#include "DownloadManager/DownloadManager.h"

AGM_Multiplayer::AGM_Multiplayer()
{
	m_bMapHasBeenQueued = false;
	bUseSeamlessTravel = true;
	m_PlayerIndexCount = 0;
}

void AGM_Multiplayer::StartGame()
{
	UGI_Adventure* GameInstance = Cast<UGI_Adventure>(GetGameInstance());
	if (GameInstance)
	{
		UE_LOG(LogNotice, Warning, TEXT("<GameMode>: Map set and is ready for game start"));
		GameInstance->StartSession();
		LoadMap(m_CurrentMapName);
	}
}

void AGM_Multiplayer::SetMapToLoad(const FString & Name)
{
	m_CurrentMapName = Name;
}

void AGM_Multiplayer::GetMapToLoad(FString & Name)const
{
	Name = m_CurrentMapName;
}

void AGM_Multiplayer::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	UE_LOG(LogNotice, Warning, TEXT("<HandleNewConnection>: New player joined!"));
	APC_Multiplayer* player = Cast<APC_Multiplayer>(NewPlayer);
	if (player)
	{
		player->SetupPacketManager();
	}
}

int AGM_Multiplayer::GeneratePlayerID()
{
	return m_PlayerIndexCount++;
}

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

bool AGM_Multiplayer::LoadMap(const FString& MapName)
{
	TActorIterator<AWorldGrid> WorldGrid(GetWorld());
	if (WorldGrid)
	{
		WorldGrid->ServerOnly_SetMapName(MapName);
	}

	FString path = FString::Printf(TEXT("%sMaps/%s.map"), *FPaths::ProjectUserDir(), *MapName);
	UMapSaveFile* Save = Cast<UMapSaveFile>(UBasicFunctions::LoadSaveGameEx(path));
	if (Save)
	{
		FString CurrentLocation = Save->ActiveLocation;

		for (auto& loc : Save->Locations)
		{
			if (loc.Name == CurrentLocation)
			{
				// Create a containter to store data that will be sent over
				ULocationSave* Location = Cast<ULocationSave>(UGameplayStatics::CreateSaveGameObject(ULocationSave::StaticClass()));
				Location->LocationData = loc;

				// Pack data into a buffer
				TArray<uint8> Buffer;
				if (UBasicFunctions::ConvertSaveToBinary(Location, Buffer))
				{
					APacketManager::ServerOnly_SetData(Buffer);

					return true;
				}

			}
		}
	}

	return false;
}
