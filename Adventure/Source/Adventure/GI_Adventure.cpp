// Fill out your copyright notice in the Description page of Project Settings.

#include "GI_Adventure.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/W_MainMenu.h"
#include "Adventure.h"
#include "UnrealNames.h"

#define SESSION_NAME EName::NAME_GameSession

const static FName MAP_MULTIPLAYER = "/Game/Maps/Multiplayer/Level_Multiplayer";
const static FName MAP_MAIN_MENU = "/Game/Maps/MainMenu/Level_MainMenu";
const static FName MAP_GAMEBUILDER = "/Game/Maps/GameBuilder/Level_GameBuilder";

UGI_Adventure::UGI_Adventure(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
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
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		UE_LOG(LogNotice, Warning, TEXT("Initializing with online subystem: %s"), *OnlineSub->GetSubsystemName().ToString());
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->OnCreateSessionCompleteDelegates.AddUObject(this, &UGI_Adventure::OnCreateOnlineSessionComplete);
			Sessions->OnStartSessionCompleteDelegates.AddUObject(this, &UGI_Adventure::OnStartOnlineSessionComplete);
			Sessions->OnDestroySessionCompleteDelegates.AddUObject(this, &UGI_Adventure::OnDestroyOnlineSessionComplete);
			Sessions->OnFindSessionsCompleteDelegates.AddUObject(this, &UGI_Adventure::OnFindOnlineSessionsComplete);
			Sessions->OnJoinSessionCompleteDelegates.AddUObject(this, &UGI_Adventure::OnJoinOnlineSessionComplete);
		}
	}

	UEngine* Engine = GetEngine();
	if (Engine)
	{
		Engine->OnNetworkFailure().AddUObject(this, &UGI_Adventure::OnNetworkFailure);
	}
}

void UGI_Adventure::Disconnect()
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->DestroySession(SESSION_NAME);
		}
	}
}

bool UGI_Adventure::JoinGame(FJOINGAME_SETTINGS settings)
{
	// Return bool
	bool bSuccessful = false;

	// Get OnlineSubsystem we want to work with
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub)
	{
		// Get SessionInterface from the OnlineSubsystem
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid())
		{
			if (SessionSearch.IsValid())
			{
				if (settings.ID >= 0 && settings.ID < SessionSearch->SearchResults.Num())
				{
					auto SearchResult = SessionSearch->SearchResults[settings.ID];
					bSuccessful = Sessions->JoinSession(0, SESSION_NAME, SearchResult);
				}
			}
		}
	}

	return bSuccessful;
}

bool UGI_Adventure::HostGame(FHOSTGAME_SETTINGS settings)
{
	// Get the Online Subsystem to work with
	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub)
	{
		// Get the Session Interface, so we can call the "CreateSession" function on it
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid())
		{
			/*
			Fill in all the Session Settings that we want to use.

			There are more with SessionSettings.Set(...);
			For example the Map or the GameMode/Type.
			*/
			SessionSettings = MakeShareable(new FOnlineSessionSettings());

			SessionSettings->bIsLANMatch = settings.IsLan;
			SessionSettings->bUsesPresence = settings.IsPresence;
			SessionSettings->NumPublicConnections = settings.MaxPlayers;
			SessionSettings->NumPrivateConnections = 0;
			SessionSettings->bAllowInvites = true;
			SessionSettings->bAllowJoinInProgress = true;
			SessionSettings->bShouldAdvertise = true;
			SessionSettings->bAllowJoinViaPresence = true;
			SessionSettings->bAllowJoinViaPresenceFriendsOnly = false;

			SessionSettings->Set(SETTING_MAPNAME, settings.MapName, EOnlineDataAdvertisementType::ViaOnlineService);

			HostGameSettings = settings;

			// Our delegate should get called when this is complete (doesn't need to be successful!)
			return Sessions->CreateSession(0 , SESSION_NAME, *SessionSettings);
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("No OnlineSubsytem found!"));
	}

	return false;
}

bool UGI_Adventure::LoadGameBuilder(FGAMEBUILDER_SETTINGS settings)
{
	FString options = "Name=" + settings.MapName + " Size=" + FString::FromInt(settings.Rows) + ":" + FString::FromInt(settings.Colums);
	UGameplayStatics::OpenLevel(GetWorld(), MAP_GAMEBUILDER, true, options);
	return true;
}

void UGI_Adventure::FindSessions(FSESSION_SEARCH_SETTINGS settings)
{

	if (!bFindingSessions)
	{
		bFindingSessions = true;
		// Get the OnlineSubsystem we want to work with
		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();

		if (OnlineSub)
		{
			// Get the SessionInterface from our OnlineSubsystem
			IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

			if (Sessions.IsValid())
			{
				/*
				Fill in all the SearchSettings, like if we are searching for a LAN game and how many results we want to have!
				*/
				SessionSearch = MakeShareable(new FOnlineSessionSearch());

				SessionSearch->bIsLanQuery = settings.IsLan;
				SessionSearch->MaxSearchResults = 20;
				SessionSearch->PingBucketSize = 50;

				// We only want to set this Query Setting if "bIsPresence" is true
				if (settings.IsPresence)
				{
					SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, settings.IsPresence, EOnlineComparisonOp::Equals);
				}

				TSharedRef<FOnlineSessionSearch> SearchSettingsRef = SessionSearch.ToSharedRef();

				// Finally call the SessionInterface function. The Delegate gets called once this is finished
				Sessions->FindSessions(0, SearchSettingsRef);
			}
		}
		else
		{
			// If something goes wrong, just call the Delegate Function directly with "false".
			OnFindOnlineSessionsComplete(false);
		}
	}
}

void UGI_Adventure::LoadMainMenu()
{
	bool success = false;

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

const TArray<FString> UGI_Adventure::GetServerList() const
{
	return SessionSearchResults;
}

bool UGI_Adventure::IsServerQueryActive()const
{
	return bFindingSessions;
}

FHOSTGAME_SETTINGS UGI_Adventure::GetHostSettings() const
{
	return HostGameSettings;
}


void UGI_Adventure::OnCreateOnlineSessionComplete(FName SessionName, bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OnCreateSessionComplete %s, %d"), *SessionName.ToString(), bWasSuccessful));

	// Get the OnlineSubsystem so we can get the Session Interface
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		// Get the Session Interface to call the StartSession function
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid())
		{
			if (bWasSuccessful)
			{
				// Our StartSessionComplete delegate should get called after this
				//FName mapName = "/Game/Maps/ThirdPersonExampleMap";
				//UGameplayStatics::OpenLevel(GetWorld(), mapName, true, "listen");
				Sessions->StartSession(SessionName);
			}
		}

	}
}

void UGI_Adventure::OnStartOnlineSessionComplete(FName SessionName, bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OnStartSessionComplete %s, %d"), *SessionName.ToString(), bWasSuccessful));

	// If the start was successful, we can open a NewMap if we want. Make sure to use "listen" as a parameter!
	if (bWasSuccessful)
	{
		if (m_mainMenu)
		{
			m_mainMenu->Deactivate();
		}

		UGameplayStatics::OpenLevel(GetWorld(), MAP_MULTIPLAYER, true, "listen");
	}
}

void UGI_Adventure::OnFindOnlineSessionsComplete(bool bWasSuccessful)
{
	bFindingSessions = false;
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OFindSessionsComplete bSuccess: %d"), bWasSuccessful));

	// Get OnlineSubsystem we want to work with
	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		// Get SessionInterface of the OnlineSubsystem
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{

			// Just debugging the Number of Search results. Can be displayed in UMG or something later on
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Num Search Results: %d"), SessionSearch->SearchResults.Num()));

			// If we have found at least 1 session, we just going to debug them. You could add them to a list of UMG Widgets, like it is done in the BP version!
			if (SessionSearch->SearchResults.Num() > 0)
			{
				// "SessionSearch->SearchResults" is an Array that contains all the information. You can access the Session in this and get a lot of information.
				// This can be customized later on with your own classes to add more information that can be set and displayed
				SessionSearchResults.Empty();
				for (int32 SearchIdx = 0; SearchIdx < SessionSearch->SearchResults.Num(); SearchIdx++)
				{
					// OwningUserName is just the SessionName for now. I guess you can create your own Host Settings class and GameSession Class and add a proper GameServer Name here.
					// This is something you can't do in Blueprint for example!

					FString SessionName;
					auto settings = SessionSearch->SearchResults[SearchIdx].Session.SessionSettings;
					if (settings.Get(SETTING_MAPNAME, SessionName))
					{
						GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Session Number: %d | Sessionname: %s "), SearchIdx + 1, *SessionName));
						SessionSearchResults.Push(SessionName + " - " + SessionSearch->SearchResults[SearchIdx].GetSessionIdStr());
					}
					else
					{
						GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Session Number: %d | Sessionname: %s "), SearchIdx + 1, *(SessionSearch->SearchResults[SearchIdx].Session.OwningUserName)));
					}
				}
			}
		}
	}
}

void UGI_Adventure::OnJoinOnlineSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type result)
{
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OnJoinSessionComplete %s, %d"), *SessionName.ToString(), static_cast<int32>(result)));

	// Get the OnlineSubsystem we want to work with
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		// Get SessionInterface from the OnlineSubsystem
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid())
		{
			// Get the first local PlayerController, so we can call "ClientTravel" to get to the Server Map
			// This is something the Blueprint Node "Join Session" does automatically!
			APlayerController * const PlayerController = GetFirstLocalPlayerController();

			// We need a FString to use ClientTravel and we can let the SessionInterface contruct such a
			// String for us by giving him the SessionName and an empty String. We want to do this, because
			// Every OnlineSubsystem uses different TravelURLs
			FString TravelURL;

			if (PlayerController && Sessions->GetResolvedConnectString(SessionName, TravelURL))
			{
				if (m_mainMenu)
				{
					m_mainMenu->Deactivate();
				}
				// Finally call the ClienTravel. If you want, you could print the TravelURL to see
				// how it really looks like
				PlayerController->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
			}
		}
	}
}

void UGI_Adventure::OnDestroyOnlineSessionComplete(FName SessionName, bool bWasSuccessful)
{
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("OnDestroySessionComplete %s, %d"), *SessionName.ToString(), bWasSuccessful));

	// Get the OnlineSubsystem we want to work with
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		// Get the SessionInterface from the OnlineSubsystem
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid())
		{
			// If it was successful, we just load another level (could be a MainMenu!)
			if (bWasSuccessful)
			{
				UGameplayStatics::OpenLevel(GetWorld(), MAP_MAIN_MENU, true);
			}
		}
	}
}

void UGI_Adventure::OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Session Failure occured")));
	Disconnect();
}
