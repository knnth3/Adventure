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

#include "GI_Adventure.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/W_MainMenu.h"
#include "Widgets/W_PauseMenu.h"
#include "UnrealNames.h"
#include "MoviePlayer.h"
#include "ViewportClient/VC_Adventure.h"
#include "basics.h"

#define SESSION_NAME EName::NAME_GameSession
#define SETTING_SESSIONID FName(TEXT("SESSION_ID"))

const static FName MAP_MAIN_MENU = "/Game/Maps/MainMenu/Level_MainMenu";
const static FName MAP_GAMEBUILDER = "/Game/Maps/GameBuilder/Level_GameBuilder";

UGI_Adventure::UGI_Adventure(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bFindingSessions = false;
}

void UGI_Adventure::Init()
{
	Super::Init();
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

	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UGI_Adventure::OnBeginLoadingScreen);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UGI_Adventure::OnEndLoadingScreen);

	bool success = false;
}

void UGI_Adventure::Disconnect()
{
	CurrentState = ADVENTURE_STATE::MAIN_MENU;

	//Fade the screen out
	const UWorld* World = GetWorld();
	if (World)
	{
		UVC_Adventure* ViewportClient = Cast<UVC_Adventure>(World->GetGameViewport());
		if (ViewportClient)
		{
			ViewportClient->Fade(0.25f, true, true);
		}
	}
}

bool UGI_Adventure::JoinGame(FJOINGAME_SETTINGS settings)
{
	JoinGameSettings = settings;
	CurrentState = ADVENTURE_STATE::CLIENT;

	//Fade the screen out
	const UWorld* World = GetWorld();
	if (World)
	{
		UVC_Adventure* ViewportClient = Cast<UVC_Adventure>(World->GetGameViewport());
		if (ViewportClient)
		{
			ViewportClient->Fade(0.25f, true, true);
		}
	}

	return true;
}

bool UGI_Adventure::HostGame(FHOSTGAME_SETTINGS settings)
{
	CurrentState = ADVENTURE_STATE::SERVER;
	HostGameSettings = settings;

	//Fade the screen out
	const UWorld* World = GetWorld();
	if (World)
	{
		UVC_Adventure* ViewportClient = Cast<UVC_Adventure>(World->GetGameViewport());
		if (ViewportClient)
		{
			ViewportClient->Fade(0.25f, true, true);
		}
	}

	return true;
}

bool UGI_Adventure::LoadGameBuilder(FGAMEBUILDER_SETTINGS settings)
{
	GameBuilderSettings = settings;
	CurrentState = ADVENTURE_STATE::GAMEBUILDER;

	//Fade the screen out
	const UWorld* World = GetWorld();
	if (World)
	{
		UVC_Adventure* ViewportClient = Cast<UVC_Adventure>(World->GetGameViewport());
		if (ViewportClient)
		{
			ViewportClient->Fade(0.25f, true, true);
		}
	}

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

				SessionSearch->bIsLanQuery = false;
				SessionSearch->MaxSearchResults = 1000000;
				//SessionSearch->PingBucketSize = 50;

				// We only want to set this Query Setting if "bIsPresence" is true
				if (true)
				{
					SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
				}

				TSharedRef<FOnlineSessionSearch> SearchSettingsRef = SessionSearch.ToSharedRef();
				FString IsLan = SessionSearch->bIsLanQuery ? "True" : "False";
				UE_LOG(LogAdventureNet, Warning, TEXT("Begin find sessions: LAN= %s, MaxResults= %i."), *IsLan, SessionSearch->MaxSearchResults);

				// Finally call the SessionInterface function. The Delegate gets called once this is finished
				Sessions->FindSessions(0, SearchSettingsRef);
			}
		}
		else
		{
			// If something goes wrong, just call the Delegate Function directly with "false".
			OnFindOnlineSessionsComplete(false);

			UE_LOG(LogAdventureNet, Error, TEXT("Unable to begin find sessions: Online Subsystem is not initialized."));
		}
	}
}

void UGI_Adventure::GetSessionList(TArray<FString>& Array) const
{
	Array = SessionSearchResults;
}

bool UGI_Adventure::IsSessionListEmpty() const
{
	return SessionSearchResults.Num() != 0;
}

bool UGI_Adventure::IsSessionSearchActive() const
{
	return bFindingSessions;
}

FHOSTGAME_SETTINGS UGI_Adventure::GetHostSettings() const
{
	return HostGameSettings;
}

void UGI_Adventure::OnBeginLoadingScreen(const FString & MapName)
{
	if (!IsRunningDedicatedServer())
	{
		const UWorld* World = GetWorld();
		if (World)
		{
			UVC_Adventure* ViewportClient = Cast<UVC_Adventure>(World->GetGameViewport());
			if (ViewportClient)
			{
				//Clear fade and show Loading screen
				ViewportClient->ClearFade();

				FLoadingScreenAttributes LoadingScreen;
				LoadingScreen.bAutoCompleteWhenLoadingCompletes = false;
				LoadingScreen.WidgetLoadingScreen = FLoadingScreenAttributes::NewTestLoadingScreenWidget();
				LoadingScreen.MinimumLoadingScreenDisplayTime = 1.0f;
				GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
			}
		}
	}
}

void UGI_Adventure::OnEndLoadingScreen(UWorld * InLoadedWorld)
{
	const UWorld* World = GetWorld();
	if (World)
	{
		UVC_Adventure* ViewportClient = Cast<UVC_Adventure>(World->GetGameViewport());
		if (ViewportClient)
		{
			if (CurrentState == ADVENTURE_STATE::CLIENT)
			{
				OnMultiplayerMapLoaded();
			}

			ViewportClient->Fade(2.0f, false);
		}
	}
}

void UGI_Adventure::OnCreateOnlineSessionComplete(FName SessionName, bool bWasSuccessful)
{
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
				UE_LOG(LogAdventureNet, Warning, TEXT("Online Session was successfully created! Starting lobby..."));
				OnLoadMultiplayerMap();
			}
			else
			{
				UE_LOG(LogAdventureNet, Error, TEXT("Create session failed for '%s'"),*SessionName.ToString());
			}
		}
		else
		{
			UE_LOG(LogAdventureNet, Error, TEXT("Create session failed: Session is invalid."));
		}

	}
	else
	{
		UE_LOG(LogAdventureNet, Error, TEXT("Create session failed: Online Subsystem not detected."));
	}
}

void UGI_Adventure::OnStartOnlineSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogNotice, Warning, TEXT("OnStartSessionComplete %s, %d"), *SessionName.ToString(), bWasSuccessful);

	// If the start was successful, we can open a NewMap if we want. Make sure to use "listen" as a parameter!
	//if (bWasSuccessful)
	//{

	//	UGameplayStatics::OpenLevel(GetWorld(), MAP_MULTIPLAYER, true, "listen");
	//}

			// Get the Online Subsystem to work with
	//IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();

	//if (OnlineSub)
	//{
	//	// Get the Session Interface, so we can call the "CreateSession" function on it
	//	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

	//	if (Sessions.IsValid())
	//	{
	//		SessionSettings->bShouldAdvertise = true;
	//		Sessions->UpdateSession(SessionName, *SessionSettings, true);
	//	}
	//}
}

void UGI_Adventure::OnFindOnlineSessionsComplete(bool bWasSuccessful)
{
	bFindingSessions = false;
	// Get OnlineSubsystem we want to work with
	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		// Get SessionInterface of the OnlineSubsystem
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			// If we have found at least 1 session, we just going to debug them. You could add them to a list of UMG Widgets, like it is done in the BP version!
			if (SessionSearch->SearchResults.Num() > 0)
			{
				// "SessionSearch->SearchResults" is an Array that contains all the information. You can access the Session in this and get a lot of information.
				// This can be customized later on with your own classes to add more information that can be set and displayed
				SessionSearchResults.Empty();
				UE_LOG(LogAdventureNet, Warning, TEXT("<FindOnlineSession>: Found %i valid sessions."), SessionSearch->SearchResults.Num());

				for (int32 SearchIdx = 0; SearchIdx < SessionSearch->SearchResults.Num(); SearchIdx++)
				{
					FString SessionName;
					auto settings = SessionSearch->SearchResults[SearchIdx].Session.SessionSettings;
					if (settings.Get(SETTING_SESSIONID, SessionName))
					{
						//ID of online service session if needed
						//SessionSearch->SearchResults[SearchIdx].GetSessionIdStr()
						SessionSearchResults.Push(SessionName);
					}
					else
					{
						UE_LOG(LogAdventureNet, Error, TEXT("<FindOnlineSession>: A session was found but it was missing map information."));
					}
				}
			}
			else
			{
				UE_LOG(LogAdventureNet, Warning, TEXT("<FindOnlineSession>: No sessions were found on search."));
			}
		}
		else
		{ 
			UE_LOG(LogAdventureNet, Error, TEXT("<FindOnlineSession>: Find sessions failed. Current online session is invalid"));
		}
	}
	else
	{
		UE_LOG(LogAdventureNet, Error, TEXT("<FindOnlineSession>: Find session failed. Online Subsystem not detected."));
	}
}

void UGI_Adventure::OnJoinOnlineSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type result)
{
	// Get the OnlineSubsystem we want to work with
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	EJoinSessionResults JoinSessionResults = UBasicFunctions::ToBlueprintType(result);
	FString JoinSessionResult = GetEnumValueAsString("EJoinSessionResults", JoinSessionResults);
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
				// Finally call the ClienTravel. If you want, you could print the TravelURL to see
				// how it really looks like
				PlayerController->ClientTravel(TravelURL, ETravelType::TRAVEL_Absolute);
				UE_LOG(LogAdventureNet, Log, TEXT("Join session success: Result='%s'"), *JoinSessionResult);
			}
			else
			{
				UE_LOG(LogAdventureNet, Warning, TEXT("Failed to join session '%s': Result='%s'"),*SessionName.ToString(), *JoinSessionResult);
			}
		}
		else
		{
			UE_LOG(LogAdventureNet, Error, TEXT("Join session failed: Session is invalid."));
		}
	}
	else
	{
		UE_LOG(LogAdventureNet, Error, TEXT("Join session failed: Online Subsystem not detected."));
	}
}

void UGI_Adventure::OnDestroyOnlineSessionComplete(FName SessionName, bool bWasSuccessful)
{
	FString IsSuccess = bWasSuccessful ? "True" : "False";
	UE_LOG(LogAdventureNet, Warning, TEXT("Attempt to destroy online session '%s': Result='%s'. Loading Main Menu..."), *SessionName.ToString(), *IsSuccess);
}

void UGI_Adventure::OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	FString FailureResult = GetEnumValueAsString("ENetworkFailure", FailureType);
	FString DriverName = NetDriver->GetFName().ToString();

	UE_LOG(LogAdventureNet, Warning, TEXT("%s: Network failure detected. Disconnecting from server: Result='%s' ErrorString='%s' "), *DriverName, *FailureResult, *ErrorString);
	Disconnect();
}

ADVENTURE_STATE UGI_Adventure::GetCurrentState() const
{
	return CurrentState;
}

void UGI_Adventure::StartSession()
{
	// Get the OnlineSubsystem so we can get the Session Interface
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		// Get the Session Interface to call the StartSession function
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

		if (Sessions.IsValid())
		{
			Sessions->StartSession(SESSION_NAME);
		}

	}
}

FHOSTGAME_SETTINGS UGI_Adventure::GetHostGameSettings()
{
	return HostGameSettings;
}

void UGI_Adventure::LoadNextMap()
{

	switch (CurrentState)
	{
	case ADVENTURE_STATE::MAIN_MENU:
	{
		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		if (OnlineSub)
		{
			IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
			if (Sessions.IsValid())
			{
				ESessionState SessionState = UBasicFunctions::ToBlueprintType(Sessions->GetSessionState(SESSION_NAME));
				FString State = GetEnumValueAsString("ESessionState", SessionState);

				UE_LOG(LogAdventureNet, Warning, TEXT("Loaing Main Menu: Session state= '%s' "), *State);

				if (SessionState != ESessionState::NoSession)
				{
					Sessions->DestroySession(SESSION_NAME);
				}

				UGameplayStatics::OpenLevel(GetWorld(), MAP_MAIN_MENU, true);
			}
		}
	}
		break;
	case ADVENTURE_STATE::GAMEBUILDER:
	{
		FString options =
			"?MN=" + GameBuilderSettings.MapName +
			"?CC=" + FString::FromInt(GameBuilderSettings.Colums) +
			"?RC=" + FString::FromInt(GameBuilderSettings.Rows) +
			"?NEW=" + (GameBuilderSettings.bNewMap ? "True" : "False");

		UGameplayStatics::OpenLevel(GetWorld(), MAP_GAMEBUILDER, true, options);
	}
		break;
	case ADVENTURE_STATE::CLIENT:
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
					if (JoinGameSettings.ID >= 0 && JoinGameSettings.ID < SessionSearch->SearchResults.Num())
					{
						auto SearchResult = SessionSearch->SearchResults[JoinGameSettings.ID];
						bSuccessful = Sessions->JoinSession(0, SESSION_NAME, SearchResult);
					}
				}
			}
		}
	}
		break;
	case ADVENTURE_STATE::SERVER:
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

				SessionSettings->bIsLANMatch = false;
				SessionSettings->bUsesPresence = true;
				SessionSettings->NumPublicConnections = 12;
				SessionSettings->NumPrivateConnections = 12;
				SessionSettings->bAllowInvites = true;
				SessionSettings->bAllowJoinInProgress = true;
				SessionSettings->bShouldAdvertise = true;
				SessionSettings->bAllowJoinViaPresence = true;
				SessionSettings->bAllowJoinViaPresenceFriendsOnly = false;
				SessionSettings->Set(SETTING_SESSIONID, HostGameSettings.SessionName, EOnlineDataAdvertisementType::ViaOnlineService);

				// Our delegate should get called when this is complete (doesn't need to be successful!)
				Sessions->CreateSession(0, SESSION_NAME, *SessionSettings);
			}
		}
		else
		{
			UE_LOG(LogNotice, Warning, TEXT("No OnlineSubsytem found!"));
		}
	}
		break;
	default:
	{
		const UWorld* World = GetWorld();
		if (World)
		{
			UVC_Adventure* ViewportClient = Cast<UVC_Adventure>(World->GetGameViewport());
			if (ViewportClient)
			{
				ViewportClient->ClearFade();
			}
		}
	}
		break;
	}

}
