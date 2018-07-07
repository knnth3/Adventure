// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.
#include "GM_Lobby.h"

#include "../Widgets/W_Lobby.h"
#include "GI_Adventure.h"

#define MULTIPLAYER_MAP "/Game/Maps/Multiplayer/Level_Multiplayer"

AGM_Lobby::AGM_Lobby()
{
	static ConstructorHelpers::FClassFinder<UW_Lobby> BP_LobbyMenu(TEXT("/Game/Blueprints/UI/Lobby/LobbyMenu"));
	if (!BP_LobbyMenu.Class)
	{
		UE_LOG(LogNotice, Error, TEXT("NO DEFAULT LOBBY MENU BLUEPRINT FOUND"));
	}

	m_playerCount = 0;
	MenuClass = BP_LobbyMenu.Class;
}

void AGM_Lobby::InitGame(const FString & MapName, const FString & Options, FString & ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	bool success = false;
	if (MenuClass)
	{
		UGI_Adventure* GameInstance = Cast<UGI_Adventure>(GetGameInstance());
		if (GameInstance)
		{
			m_LobbyMenu = CreateWidget<UW_Lobby>(GameInstance, MenuClass);
			if (m_LobbyMenu)
			{
				m_LobbyMenu->AddServerCallback(this);
				success = m_LobbyMenu->Activate();
			}
		}
	}

}

void AGM_Lobby::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	UWorld* world = GetWorld();
	if (world && m_LobbyMenu && m_playerCount)
	{
		m_LobbyMenu->AddCharacter("Player " + FString::FromInt((int)m_playerCount));
	}

	m_playerCount++;
}

void AGM_Lobby::StartGame()
{
	UGI_Adventure* GameInstance = Cast<UGI_Adventure>(GetGameInstance());
	UWorld* World = GetWorld();
	if (World && GameInstance)
	{
		bUseSeamlessTravel = true;
		World->ServerTravel(MULTIPLAYER_MAP + FString("?listen"));
		GameInstance->StartSession();
		OnGameStart();
	}
}

