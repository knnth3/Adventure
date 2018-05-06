// Fill out your copyright notice in the Description page of Project Settings.

#include "GI_Adventure.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/W_MainMenu.h"
#include "OnlineSessionSettings.h"
#include "Adventure.h"

#define TEMP_SESSION_NAME "GAME_TEST2"

UGI_Adventure::UGI_Adventure()
{
	static ConstructorHelpers::FClassFinder<UW_MainMenu> MainMenu(TEXT("/Game/Blueprints/UI/MainMenu/MainMenu"));
	if (MainMenu.Class != nullptr)
	{
		MenuClass = MainMenu.Class;
	}

	m_mainMenu = nullptr;
	bFindingSessions = false;
}

void UGI_Adventure::Init()
{
	IOnlineSubsystem* subsystem =  IOnlineSubsystem::Get();
	if (subsystem)
	{
		UE_LOG(LogNotice, Warning, TEXT("Initializing with online subystem: %s"), *subsystem->GetSubsystemName().ToString());
		m_onlineSession = subsystem->GetSessionInterface();
		if (m_onlineSession.IsValid())
		{
			m_onlineSession->OnCreateSessionCompleteDelegates.AddUObject(this, &UGI_Adventure::OnCreateSession);
			m_onlineSession->OnDestroySessionCompleteDelegates.AddUObject(this, &UGI_Adventure::OnRemoveSession);
			m_onlineSession->OnFindSessionsCompleteDelegates.AddUObject(this, &UGI_Adventure::OnFoundSessions);

			//TDO: move to somewhere else
			m_sessionSearch = MakeShareable(new FOnlineSessionSearch());
			if (m_sessionSearch.IsValid())
			{
				m_sessionSearch->bIsLanQuery = true;
				//m_sessionSearch->QuerySettings.Set();

				m_onlineSession->FindSessions(0, m_sessionSearch.ToSharedRef());
				bFindingSessions = true;
			}
		}
	}
}

void UGI_Adventure::OnCreateSession(FName name, bool success)
{
	if (success)
	{
		if (m_mainMenu)
		{
			m_mainMenu->Deactivate();
		}

		//Start Host functions
		UWorld* World = GetWorld();
		if (World)
		{
			FString mapName = "ThirdPersonExampleMap";
			World->ServerTravel("/Game/Maps/" + mapName + "?listen");
		}
	}
}

void UGI_Adventure::OnRemoveSession(FName name, bool success)
{
	UE_LOG(LogNotice, Warning, TEXT("Sassion closed: %s"), *name.ToString());
}

void UGI_Adventure::OnFoundSessions(bool success)
{
	if (success)
	{
		UE_LOG(LogNotice, Warning, TEXT("Sessions Found:"));

		for (const auto& session : m_sessionSearch->SearchResults)
		{
			m_sessionSearchResults.Push(session.GetSessionIdStr());
		}
	}

	bFindingSessions = false;
}

void UGI_Adventure::Disconnect()
{
	APlayerController* controller = GetFirstLocalPlayerController();
	if (controller)
	{
		FString mapName = "MainMenu/Level_MainMenu";
		controller->ClientTravel("/Game/Maps/" + mapName, ETravelType::TRAVEL_Absolute);
	}
}

void UGI_Adventure::JoinGame(const FString address)
{
	if (m_mainMenu)
	{
		m_mainMenu->Deactivate();
	}

	APlayerController* controller = GetFirstLocalPlayerController();
	if (controller)
	{
		controller->ClientTravel(address, ETravelType::TRAVEL_Absolute);
	}
}

void UGI_Adventure::HostGame(const FString map)
{
	if (m_onlineSession.IsValid())
	{
		FOnlineSessionSettings settings;
		settings.bIsLANMatch = true;
		settings.NumPublicConnections = 5;
		settings.bShouldAdvertise = true;

		m_onlineSession->CreateSession(0, TEXT(TEMP_SESSION_NAME), settings);
	}
}

void UGI_Adventure::LoadMainMenu()
{
	bool success = false;
	if (m_onlineSession.IsValid())
	{
		m_onlineSession->RemoveNamedSession(TEXT(TEMP_SESSION_NAME));
	}
	if (MenuClass)
	{
		m_mainMenu = CreateWidget<UW_MainMenu>(this, MenuClass);
		if (m_mainMenu)
		{
			m_mainMenu->AddCallbackInterface(this);
			success = m_mainMenu->Activate();
		}
	}
}

const TArray<FString> UGI_Adventure::GetSessionSearchResults() const
{
	return m_sessionSearchResults;
}

bool UGI_Adventure::IsFindingSessions() const
{
	return bFindingSessions;
}
