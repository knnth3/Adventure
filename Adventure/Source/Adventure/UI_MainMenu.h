// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UI_MainMenu.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UUI_MainMenu : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */

class ADVENTURE_API IUI_MainMenu
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	IUI_MainMenu();

	virtual void JoinGame(const FString address) = 0;
	virtual void HostGame(const FString map) = 0;
	
	
};
