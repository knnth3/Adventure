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

USTRUCT(BlueprintType)
struct FGAMEBUILDER_SETTINGS
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameBuilder Settings")
	int Rows = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameBuilder Settings")
	int Colums = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameBuilder Settings")
	bool bNewMap = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameBuilder Settings")
	FString MapName = "";
};

USTRUCT(BlueprintType)
struct FJOINGAME_SETTINGS
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Join Game Settings")
	int Port = 1234;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Join Game Settings")
	FString Address = "";
};

USTRUCT(BlueprintType)
struct FHOSTGAME_SETTINGS
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Host Game Settings")
	int MaxPlayers = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Host Game Settings")
	FString MapName = "";
};

UCLASS()
class ADVENTURE_API UW_MainMenu : public UUserWidget
{
	GENERATED_BODY()
	
	friend class UW_MainMenu_Child;
protected:

	virtual bool Initialize()override;


	//State request functions
	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void SetGameBuilderSettings(FGAMEBUILDER_SETTINGS settings);

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void SetJoinGameSettings(FJOINGAME_SETTINGS settings);

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void SetHostGameSettings(FHOSTGAME_SETTINGS settings);

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void CloseGame();

	//Navigation functions

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void ShowHostMenu();

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void ShowJoinMenu();

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void ShowGameBuilderMenu();

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void ShowHomeMenu();

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void ShowCustom();

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void SetHostGameMenuClass(TSubclassOf<class UW_MainMenu_Child> HostMenuClass);

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void SetJoinGameMenuClass(TSubclassOf<class UW_MainMenu_Child> JoinMenuClass);

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void SetGameBuilderMenuClass(TSubclassOf<class UW_MainMenu_Child> GameBuilderMenuClass);

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void SetHomeMenuClass(TSubclassOf<class UW_MainMenu_Child> GameBuilderMenuClass);

	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* SubMenu;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* Play;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* Exit;

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
	FHOSTGAME_SETTINGS m_hostSettings;
	FJOINGAME_SETTINGS m_joinSettings;
	FGAMEBUILDER_SETTINGS m_gbSettings;
	IUI_MainMenu* m_interface = nullptr;
	class UW_MainMenu_Child* m_homeMenu = nullptr;
	class UW_MainMenu_Child* m_hostMenu = nullptr;
	class UW_MainMenu_Child* m_joinMenu = nullptr;
	class UW_MainMenu_Child* m_gameBuilderMenu = nullptr;
};