// Fill out your copyright notice in the Description page of Project Settings.

#include "W_MainMenu_Child.h"
#include "W_MainMenu.h"


void UW_MainMenu_Child::ConnectTo(UW_MainMenu* MainMenu)
{
	m_mainMenu = MainMenu;
}

void UW_MainMenu_Child::RequestHostGame(const FString mapName)
{
}

void UW_MainMenu_Child::RequestJoinGame(const FString mapName)
{
}

void UW_MainMenu_Child::RequestLaunchGameBuilder(const bool newMap, const int rows, const int columns, const FString mapName)
{
	if (m_mainMenu)
	{
		GAMEBUILDER_SETTINGS settings;
		settings.bNewMap = newMap;
		settings.Rows = rows;
		settings.Colums = columns;
		settings.MapName = mapName;
		m_mainMenu->SetGameBuilderSettings(settings);
	}
}
