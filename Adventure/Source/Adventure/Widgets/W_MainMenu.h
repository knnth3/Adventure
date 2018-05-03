// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI_MainMenu.h"
#include "Blueprint/UserWidget.h"
#include "W_MainMenu.generated.h"

/**
 * 
 */

enum class ACTIVE_MENU
{
	HOME,
	JOIN,
	HOST,
	GAMEBUILDER,
	CUSTOM
};

struct GAMEBUILDER_SETTINGS
{
	int Rows;
	int Colums;
	bool bNewMap;
	FString MapName;
};

UCLASS()
class ADVENTURE_API UW_MainMenu : public UUserWidget
{
	GENERATED_BODY()
	
	friend class UW_MainMenu_Child;
protected:

	virtual bool Initialize()override;
	void SetGameBuilderSettings(GAMEBUILDER_SETTINGS settings);

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void ShowHostMenu();

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void ShowJoinMenu();

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void ShowGameBuilderMenu();

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void SetHostGameMenuClass(TSubclassOf<class UW_MainMenu_Child> HostMenuClass);

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void SetJoinGameMenuClass(TSubclassOf<class UW_MainMenu_Child> JoinMenuClass);

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void SetGameBuilderMenuClass(TSubclassOf<class UW_MainMenu_Child> GameBuilderMenuClass);

	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* SubMenu;

	UPROPERTY(meta = (BindWidget))
	class UButton* Play;

public:

	void AddCallbackInterface(IUI_MainMenu* Interface);
	bool Activate();
	void Deactivate();

private:

	UFUNCTION()
	void LoadNextState();

	void HostGame(const FString mapName);
	void JoinGame(const FString address = "127.0.0.1");
	void LoadEditor(const FString mapName = "");

	ACTIVE_MENU m_active;
	GAMEBUILDER_SETTINGS m_gbSettings;
	IUI_MainMenu* m_interface = nullptr;
	class UW_MainMenu_Child* m_hostMenu = nullptr;
	class UW_MainMenu_Child* m_joinMenu = nullptr;
	class UW_MainMenu_Child* m_gameBuilderMenu = nullptr;
};