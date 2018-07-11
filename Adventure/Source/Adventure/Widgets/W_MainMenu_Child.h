// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI_MainMenu.h"
#include "Blueprint/UserWidget.h"
#include "W_MainMenu_Child.generated.h"

/**
 * 
 */
UCLASS()
class ADVENTURE_API UW_MainMenu_Child : public UUserWidget
{
	GENERATED_BODY()
	
public:

	void ConnectTo(class UW_MainMenu* MainMenu);

protected:

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void RequestHostGame(FHOSTGAME_SETTINGS settings);

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void RequestJoinGame(FJOINGAME_SETTINGS settings);

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void RequestLaunchGameBuilder(FGAMEBUILDER_SETTINGS settings);

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	const TArray<FString> GetServerList()const;

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	virtual bool IsSessionSearchActive()const;

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	bool GetAllSaveGameSlotNames(TArray<FString>& Array, FString Ext);

private:

	class UW_MainMenu* MainMenu = nullptr;
	
};
