// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Adventure.h"
#include "UI_MainMenu.h"
#include "Engine/GameInstance.h"
#include "GI_Adventure.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class ADVENTURE_STATE : uint8
{
	MAIN_MENU,
	GAMEBUILDER,
	CLIENT,
	SERVER,
};

UCLASS()
class ADVENTURE_API UGI_Adventure : public UGameInstance, public IUI_MainMenu
{
	GENERATED_BODY()

public:

	UGI_Adventure(const FObjectInitializer& ObjectInitializer);
	virtual void Init()override;

	UFUNCTION(BlueprintCallable)
	void Disconnect();

	UFUNCTION(BlueprintCallable)
	virtual bool JoinGame(FJOINGAME_SETTINGS settings)override;

	UFUNCTION(BlueprintCallable)
	virtual bool HostGame(FHOSTGAME_SETTINGS settings)override;

	UFUNCTION(BlueprintCallable)
	virtual bool LoadGameBuilder(FGAMEBUILDER_SETTINGS settings)override;

	UFUNCTION(BlueprintCallable)
	virtual void FindSessions(FSESSION_SEARCH_SETTINGS settings)override;

	UFUNCTION(BlueprintCallable)
	const TArray<FString> GetServerList()const;

	UFUNCTION(BlueprintCallable)
	virtual bool IsServerQueryActive()const;

	UFUNCTION(BlueprintCallable)
	FHOSTGAME_SETTINGS GetHostSettings()const;

	UFUNCTION(BlueprintCallable)
	virtual void BeginLoadingScreen(const FString& MapName);

	UFUNCTION(BlueprintCallable)
	virtual void EndLoadingScreen(UWorld* InLoadedWorld);

	UFUNCTION(BlueprintCallable)
	virtual void LoadMainMenu() override;

	UFUNCTION(BlueprintCallable)
	ADVENTURE_STATE GetCurrentState()const;

	UFUNCTION(BlueprintCallable)
	void StartSession();

	UFUNCTION(BlueprintCallable)
	FHOSTGAME_SETTINGS GetHostGameSettings();

	void LoadNextMap();

protected:

	//Online session callbacks
	void OnCreateOnlineSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartOnlineSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindOnlineSessionsComplete(bool bWasSuccessful);
	void OnJoinOnlineSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type result);
	void OnDestroyOnlineSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);

private:

	//Online Sessions
	bool bFindingSessions;
	ADVENTURE_STATE CurrentState = ADVENTURE_STATE::MAIN_MENU;
	FHOSTGAME_SETTINGS HostGameSettings;
	FJOINGAME_SETTINGS JoinGameSettings;
	FGAMEBUILDER_SETTINGS GameBuilderSettings;
	TArray<FString> SessionSearchResults;
	TSharedPtr<class FOnlineSessionSearch> SessionSearch;
	TSharedPtr<class FOnlineSessionSettings> SessionSettings;
};
