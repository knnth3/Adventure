// Fill out your copyright notice in the Description page of Project Settings.
#include "W_MainMenu_Child.h"

#include "GI_Adventure.h"
#include "W_MainMenu.h"
#include "FileManager.h"
#include "Paths.h"

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
	if (m_mainMenu && settings.Colums && settings.Rows && !settings.MapName.IsEmpty())
	{
		m_mainMenu->SetGameBuilderSettings(settings);
		m_mainMenu->LoadNextState();
	}
	else
	{
		UE_LOG(LogNotice, Warning, TEXT("Request to launch GameBuilder Failed. Main Menu: %i, Columns: %i, Rows: %i, MapName: %s"), 
			(m_mainMenu != 0), settings.Colums, settings.Rows, *settings.MapName);
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

bool UW_MainMenu_Child::GetAllSaveGameSlotNames(TArray<FString>& Array, FString Ext)
{
	FString RootFolderFullPath = FPaths::ProjectSavedDir() + "/SaveGames/";

	if (RootFolderFullPath.Len() < 1) return false;

	FPaths::NormalizeDirectoryName(RootFolderFullPath);

	UE_LOG(LogNotice, Warning, TEXT("Attempting to locate all filed in folderpath: %s"), *RootFolderFullPath);

	IFileManager& FileManager = IFileManager::Get();

	if (Ext == "")
	{
		Ext = "*.*";
	}
	else
	{
		Ext = (Ext.Left(1) == ".") ? "*" + Ext : "*." + Ext;
	}

	FString FinalPath = RootFolderFullPath + "/" + Ext;
	FileManager.FindFiles(Array, *FinalPath, true, false);
	return true;
}