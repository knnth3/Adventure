// Fill out your copyright Warning in the Description page of Project Settings.

#include "GM_Adventure_MainMenu.h"
#include "Character/MapPawn.h"
#include "UObject/ConstructorHelpers.h"
#include "Adventure.h"

AGM_Adventure_MainMenu::AGM_Adventure_MainMenu()
{
	UE_LOG(LogNotice, Display, TEXT("Using GameMode:  %s"), *this->GetName());
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Characters/MapPawn/ControllableMapPawn"));
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerController(TEXT("/Script/Adventure.PC_Adventure_Default"));

	if (PlayerPawnBPClass.Class == nullptr)
	{
		UE_LOG(LogNotice, Error, TEXT("NO DEFAULT PLAYER PAWN CLASS FOUND"));
	}

	if (PlayerController.Class == nullptr)
	{
		UE_LOG(LogNotice, Error, TEXT("NO DEFAULT PLAYER CONTROLLER CLASS FOUND"));
	}

	DefaultPawnClass = PlayerPawnBPClass.Class;
	PlayerControllerClass = PlayerController.Class;
}

