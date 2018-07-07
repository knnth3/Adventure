// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "W_Lobby.h"
#include "GameModes/GM_Lobby.h"


bool UW_Lobby::Initialize()
{
	bool Init = Super::Initialize();
	if (Init)
	{
		m_Gamemode = nullptr;
	}
	return Init;
}

bool UW_Lobby::Activate()
{
	bool success = false;
	AddToViewport();

	return success;
}

void UW_Lobby::Deactivate()
{
	RemoveFromViewport();
}

void UW_Lobby::AddCharacter(FString Username)
{
	OnCharacterConnected(Username);
}

void UW_Lobby::AddServerCallback(AGM_Lobby * Gamemode)
{
	m_Gamemode = Gamemode;
}

void UW_Lobby::StartSession()
{
	if (m_Gamemode)
	{
		m_Gamemode->StartGame();
	}
}

void UW_Lobby::SetMapToLoad(const FString & Name)
{
	if (m_Gamemode)
	{
		m_Gamemode->SetMapToLoad(Name);
	}
}

bool UW_Lobby::IsMapSelected()const
{
	if (m_Gamemode)
	{
		FString MapName;
		m_Gamemode->GetMapToLoad(MapName);

		return !MapName.IsEmpty();
	}
	return false;
}
