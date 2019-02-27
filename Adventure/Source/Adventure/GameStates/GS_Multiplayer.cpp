// Fill out your copyright notice in the Description page of Project Settings.
#include "GS_Multiplayer.h"

#include "Grid/WorldGrid.h"
#include "Character/MapPawn.h"
#include "UObject/ConstructorHelpers.h"
#include "GameModes/GM_Multiplayer.h"
#include "PlayerStates/PS_Multiplayer.h"
#include "DataTables/InventoryDatabase.h"
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
	DOREPLIFETIME(AGS_Multiplayer, m_PlayerNameArray);
}

void AGS_Multiplayer::HandleBeginPlay()
{
	Super::HandleBeginPlay();
	//SetActivePlayer(-1);
	GenerateGrid();
}

void AGS_Multiplayer::AddNewPlayer(int PlayerID, FString PlayerName)
{
	// Validity check
	if (m_PlayerNameArray.Num() == PlayerID)
	{
		m_PlayerNameArray.Push(PlayerName);
	}
}

void AGS_Multiplayer::GenerateGrid()
{
	if (HasAuthority())
	{
		UE_LOG(LogNotice, Warning, TEXT("<GameState>: Generate Grid"));
		bool bGenerateNewMap = false;
		// Ensure the map knows a a given map should be created
		AGM_Multiplayer* Gamemode = Cast<AGM_Multiplayer>(AuthorityGameMode);
		if (Gamemode)
		{
			TActorIterator<AWorldGrid> WorldGrid(GetWorld());
			if (WorldGrid)
			{
				WorldGrid->ServerOnly_SetMapName(Gamemode->GetMapName());
				WorldGrid->ServerOnly_SetMapSize(Gamemode->GetMapSize());

				APS_Multiplayer* PS = Cast<APS_Multiplayer>(PlayerArray[0]);
				if (PS && !PS->ServerOnly_LoadMap(Gamemode->GetMapName()))
				{
					PS->GenerateEmptyMap(WorldGrid->GetMapName(), Gamemode->GetMapSize());
				}
			}

			FString path = FString::Printf(TEXT("%sMaps/%s.map"), *FPaths::ProjectUserDir(), *Gamemode->GetMapName());
			if (!FPaths::FileExists(path))
			{
				FWeaponInfo Winfo;
				Winfo.Name = TEXT("Basic Sword");
				Winfo.Description = TEXT("Ye ol faithful");
				Winfo.VisualIndex = 1;
				UInventoryDatabase::AddWeaponToDatabase(Winfo);

				FConsumableInfo Cinfo;
				Cinfo.Name = TEXT("Cake");
				Cinfo.Description = TEXT("Delicious!");
				Cinfo.HealthBonus = 1;
				UInventoryDatabase::AddConsumableToDatabase(Cinfo);

				bGenerateNewMap = true;
			}
		}
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

FString AGS_Multiplayer::GetPlayerName(int PlayerID) const
{
	if (m_PlayerNameArray.Num() > PlayerID)
	{
		return m_PlayerNameArray[PlayerID];
	}

	return "None";
}

TArray<FString> AGS_Multiplayer::GetAllPlayerNames() const
{
	return m_PlayerNameArray;
}

int AGS_Multiplayer::GetPlayerID(FString PlayerName) const
{
	int ID = -1;

	for (int x = 0; x < m_PlayerNameArray.Num(); x++)
	{
		if (m_PlayerNameArray[x] == PlayerName)
		{
			ID = x;
		}
	}

	return ID;
}

void AGS_Multiplayer::ServerOnly_LoadMapOnClients(const FString& MapName) const
{
	for (int x = 1; x < PlayerArray.Num(); x++)
	{

		APS_Multiplayer* MPS = Cast<APS_Multiplayer>(PlayerArray[x]);
		if (MPS)
		{
			MPS->LoadMap(MapName);
		}
	}
}
