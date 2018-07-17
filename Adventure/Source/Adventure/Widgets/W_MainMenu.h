// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <memory>
#include "CoreMinimal.h"
#include "GI_Adventure.h"
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

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	bool HostGame(const FHOSTGAME_SETTINGS& Settings);

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	bool JoinGame(const FJOINGAME_SETTINGS& Settings);

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	bool LoadGameBuilder(const FGAMEBUILDER_SETTINGS& Settings);

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void CloseGame();

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void ActivateSubMenu(int index);

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void RefreshServerList(const FSESSION_SEARCH_SETTINGS & Settings);

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void GetSessionList(TArray<FString>& Array) const;

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	bool IsSessionSearchActive() const;

	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* SubMenu;

	UPROPERTY(EditAnywhere, Category = "Sub-Menus")
	TArray<TSubclassOf<class UW_MainMenu_Child>> SubMenuClasses;

private:

	void CreateSubMenus();

	UGI_Adventure* GameInstance;
};