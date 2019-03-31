// Copyright 2019 Eric Marquez
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
// http ://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "W_MainMenu_Child.h"

#include "GI_Adventure.h"
#include "W_MainMenu.h"
#include "FileManager.h"
#include "Paths.h"

void UW_MainMenu_Child::ConnectTo(UW_MainMenu* mainMenu)
{
	this->MainMenu = mainMenu;
}

void UW_MainMenu_Child::RequestHostGame(FHOSTGAME_SETTINGS settings)
{
	if (!settings.SessionName.IsEmpty())
	{
		MainMenu->HostGame(settings);
	}
	else
	{
		UE_LOG(LogNotice, Warning, TEXT("<HostSessionRequest>: Failed to host session. Session name was NULL."));
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