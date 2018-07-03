// Fill out your copyright notice in the Description page of Project Settings.

#include "W_MainMenu_Child.h"
#include "W_MainMenu.h"

void UW_MainMenu_Child::ConnectTo(UW_MainMenu* MainMenu)
{
	m_mainMenu = MainMenu;
}

void UW_MainMenu_Child::RequestHostGame(FHOSTGAME_SETTINGS settings)
{
	if (m_mainMenu)
	{
		if (!settings.SessionName.IsEmpty() && !settings.MapName.IsEmpty())
		{
			m_mainMenu->SetHostGameSettings(settings);
			m_mainMenu->LoadNextState();
		}
	}
}

void UW_MainMenu_Child::RequestJoinGame(FJOINGAME_SETTINGS settings)
{
	if (m_mainMenu)
	{
		m_mainMenu->SetJoinGameSettings(settings);
		m_mainMenu->LoadNextState();
	}
}

void UW_MainMenu_Child::RequestLaunchGameBuilder(FGAMEBUILDER_SETTINGS settings)
{
	if (m_mainMenu)
	{
		m_mainMenu->SetGameBuilderSettings(settings);
	}
}

const TArray<FString> UW_MainMenu_Child::GetServerList() const
{
	if (m_mainMenu)
	{
		return m_mainMenu->GetServerList();
	}
	return TArray<FString>();
}

bool UW_MainMenu_Child::IsServerQueryActive()const
{
	if (m_mainMenu)
	{
		return m_mainMenu->IsServerQueryActive();
	}
	return false;
}
