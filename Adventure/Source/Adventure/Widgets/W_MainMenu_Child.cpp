// Fill out your copyright notice in the Description page of Project Settings.
#include "W_MainMenu_Child.h"

#include "GI_Adventure.h"
#include "W_MainMenu.h"
#include "FileManager.h"
#include "Paths.h"

void UW_MainMenu_Child::ConnectTo(UW_MainMenu* MainMenu)
{
	this->MainMenu = MainMenu;
}

void UW_MainMenu_Child::RequestHostGame(FHOSTGAME_SETTINGS settings)
{
	if (MainMenu && !settings.SessionName.IsEmpty() && !settings.MapName.IsEmpty())
	{
		MainMenu->HostGame(settings);
	}
	else
	{
		FString HasMainMenu = (MainMenu != 0) ? "True" : "False";
		UE_LOG(LogNotice, Warning, TEXT("Request to host game failed: Main Menu Loaded= '%s', Session Name= '%s', Map Name: '%s'"),
			*HasMainMenu, *settings.SessionName, *settings.MapName);
	}
}

void UW_MainMenu_Child::RequestJoinGame(FJOINGAME_SETTINGS settings)
{
	if (MainMenu)
	{
		if (!MainMenu->JoinGame(settings))
		{
			UE_LOG(LogNotice, Warning, TEXT("Request to join game failed: Session ID is invalid."));
		}
	}
}

void UW_MainMenu_Child::RequestLaunchGameBuilder(FGAMEBUILDER_SETTINGS settings)
{
	if (MainMenu && !settings.bNewMap && !settings.MapName.IsEmpty())
	{
		MainMenu->LoadGameBuilder(settings);
	}
	else if (MainMenu && settings.Colums && settings.Rows && !settings.MapName.IsEmpty())
	{
		MainMenu->LoadGameBuilder(settings);
	}
	else
	{
		FString HasMainMenu = (MainMenu != 0) ? "True" : "False";
		UE_LOG(LogNotice, Warning, TEXT("Request to launch GameBuilder Failed: Main Menu Loaded= '%s', MapName= '%s', Columns= '%i', Rows= '%i'"), 
			*HasMainMenu, *settings.MapName, settings.Colums, settings.Rows);
	}
}

void UW_MainMenu_Child::GetServerList(TArray<FString>& Array) const
{
	if (MainMenu)
	{
		return MainMenu->GetSessionList(Array);
	}
}

bool UW_MainMenu_Child::IsSessionSearchActive()const
{
	if (MainMenu)
	{
		return MainMenu->IsSessionSearchActive();
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