// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UI_MainMenu.h"
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionInterface.h"
#include "GI_Adventure.generated.h"

/**
 * 
 */
UCLASS()
class ADVENTURE_API UGI_Adventure : public UGameInstance, public IUI_MainMenu
{
	GENERATED_BODY()

public:

	UGI_Adventure();
	virtual void Init()override;

	//Online session callbacks
	void OnCreateSession(FName name, bool success);
	void OnRemoveSession(FName name, bool success);
	void OnFoundSessions(bool success);

	UFUNCTION(BlueprintCallable)
	void Disconnect();

	UFUNCTION(BlueprintCallable)
	void JoinGame(const FString address);

	UFUNCTION(BlueprintCallable)
	void HostGame(const FString map);

	UFUNCTION(BlueprintCallable)
	void LoadMainMenu();

	UFUNCTION(BlueprintCallable)
	const TArray<FString> GetSessionSearchResults()const;

	UFUNCTION(BlueprintCallable)
	bool IsFindingSessions()const;

private:

	bool bFindingSessions;
	class UW_MainMenu* m_mainMenu;
	IOnlineSessionPtr m_onlineSession;
	TSubclassOf<class UW_MainMenu> MenuClass;
	TArray<FString> m_sessionSearchResults;
	TSharedPtr<FOnlineSessionSearch> m_sessionSearch;
};
