// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UI_MainMenu.h"
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GM_Adventure_MainMenu.generated.h"

/**
 * 
 */
UCLASS()
class ADVENTURE_API AGM_Adventure_MainMenu : public AGameModeBase, public IUI_MainMenu
{
	GENERATED_BODY()
	
public:

	AGM_Adventure_MainMenu();

	UFUNCTION(BlueprintCallable)
	void LoadMenu();

	
protected:

	virtual void JoinGame(const FString address)override;
	virtual void HostGame(const FString map)override;


private:

	class UW_MainMenu* m_mainMenu = nullptr;
	TSubclassOf<class UW_MainMenu> MenuClass;
};
