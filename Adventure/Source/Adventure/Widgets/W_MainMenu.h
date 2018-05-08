// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <memory>
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
	void RefreshServerList();

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	const TArray<FString> GetServerList()const;

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	virtual bool IsServerQueryActive()const;

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

	void HostGame();
	void JoinGame();
	void LoadEditor();

	ACTIVE_MENU m_active;
	IUI_MainMenu* m_interface = nullptr;
	class UW_MainMenu_Child* m_homeMenu = nullptr;
	class UW_MainMenu_Child* m_hostMenu = nullptr;
	class UW_MainMenu_Child* m_joinMenu = nullptr;
	class UW_MainMenu_Child* m_gameBuilderMenu = nullptr;
	std::unique_ptr<FHOSTGAME_SETTINGS> m_hostSettings;
	std::unique_ptr<FJOINGAME_SETTINGS> m_joinSettings;
	std::unique_ptr<FGAMEBUILDER_SETTINGS> m_gbSettings;
};