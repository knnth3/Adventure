// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UI_MainMenu.generated.h"


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
	FName Username = "Anonymous";
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Join Game Settings")
	int Port = 1234;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Join Game Settings")
	int ID = -1;
};

USTRUCT(BlueprintType)
struct FHOSTGAME_SETTINGS
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Host Game Settings")
	int32 MaxPlayers = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Host Game Settings")
	FString MapName = "NONE";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Host Game Settings")
	FString SessionName = "GAME_MULTIPLAYER";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Host Game Settings")
	bool IsLan = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Host Game Settings")
	bool IsPresence = true;
};

USTRUCT(BlueprintType)
struct FSESSION_SEARCH_SETTINGS
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Host Game Settings")
	bool IsLan = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Host Game Settings")
	bool IsPresence = true;
};

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

	virtual bool JoinGame(FJOINGAME_SETTINGS settings) = 0;
	virtual bool HostGame(FHOSTGAME_SETTINGS settings) = 0;
	virtual bool LoadGameBuilder(FGAMEBUILDER_SETTINGS settings) = 0;
	virtual void FindSessions(FSESSION_SEARCH_SETTINGS settings) = 0;
	virtual const TArray<FString> GetServerList()const = 0;
	virtual bool IsServerQueryActive()const = 0;
	
};
