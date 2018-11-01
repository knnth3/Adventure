// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.
#include "GM_Lobby.h"

#include "GI_Adventure.h"
#include "Widgets/HUD_MPLobby.h"

#define MULTIPLAYER_MAP "/Game/Maps/Multiplayer/Level_Multiplayer"

AGM_Lobby::AGM_Lobby()
{
	m_playerCount = 0;
}

void AGM_Lobby::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	UWorld* world = GetWorld();

	APlayerController* Controller = UGameplayStatics::GetPlayerController(world, 0);
	if (Controller)
	{
		AHUD_MPLobby* HUD = Cast<AHUD_MPLobby>(Controller->GetHUD());
		if (HUD)
		{
			HUD->AddCharacter(NewPlayer->PlayerState->GetPlayerName());
		}
	}

	m_playerCount++;
}

void AGM_Lobby::SetMapToLoad(const FString & Name)
{
	m_MapSaveName = Name;
}

void AGM_Lobby::GetMapToLoad(FString & Name)const
{
	Name = m_MapSaveName;
}

void AGM_Lobby::StartGame()
{
	UGI_Adventure* GameInstance = Cast<UGI_Adventure>(GetGameInstance());
	UWorld* World = GetWorld();
	if (World && GameInstance)
	{
		FString Option1 = TEXT("?listen");
		FString Option2 = TEXT("?SN=") + m_MapSaveName;
		FString URL = FString(MULTIPLAYER_MAP) + Option1 + Option2;
		bUseSeamlessTravel = true;
		World->ServerTravel(URL);
		GameInstance->StartSession();
		OnGameStart();
	}
}

