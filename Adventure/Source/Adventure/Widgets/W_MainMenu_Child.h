// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
	void RequestHostGame(const FString mapName);

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void RequestJoinGame(const FString address = "127.0.0.1", const int port = 1234);

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void RequestLaunchGameBuilder(const bool newMap = true, const int rows = 100, const int columns = 100, const FString mapName = "");

private:

	class UW_MainMenu* m_mainMenu = nullptr;
	
};
