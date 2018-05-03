// Fill out your copyright Warning in the Description page of Project Settings.

#include "GM_Adventure_MainMenu.h"
#include "AdventureCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Widgets/W_MainMenu.h"
#include "Adventure.h"

AGM_Adventure_MainMenu::AGM_Adventure_MainMenu()
{
	UE_LOG(LogNotice, Display, TEXT("Using GameMode:  %s"), *this->GetName());
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Characters/BasicCharacter/ThirdPersonCharacter"));
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerController(TEXT("/Script/Adventure.PC_Adventure_Default"));
	static ConstructorHelpers::FClassFinder<UW_MainMenu> MainMenu(TEXT("/Game/Blueprints/UI/MainMenu/MainMenuTemp"));

	if (PlayerPawnBPClass.Class == nullptr)
	{
		UE_LOG(LogNotice, Error, TEXT("NO DEFAULT PLAYER PAWN CLASS FOUND"));
	}

	if (PlayerController.Class == nullptr)
	{
		UE_LOG(LogNotice, Error, TEXT("NO DEFAULT PLAYER CONTROLLER CLASS FOUND"));
	}

	if (MainMenu.Class == nullptr)
	{
		UE_LOG(LogNotice, Error, TEXT("NO DEFAULT MAIN MENU CLASS FOUND"));
	}

	MenuClass = MainMenu.Class;
	DefaultPawnClass = PlayerPawnBPClass.Class;
	PlayerControllerClass = PlayerController.Class;
}

void AGM_Adventure_MainMenu::LoadMenu()
{
	bool success = false;
	if (MenuClass)
	{
		UGameInstance* GameInstance = GetGameInstance();
		if (GameInstance)
		{
			m_mainMenu = CreateWidget<UW_MainMenu>(GameInstance, MenuClass);
			if (m_mainMenu)
			{
				m_mainMenu->AddCallbackInterface(this);
				success = m_mainMenu->Activate();
			}
		}
	}
	if (!success)
	{
		UE_LOG(LogNotice, Error, TEXT("MENU WAS NOT LOADED SUCCESSFULLY"));
	}
}

void AGM_Adventure_MainMenu::JoinGame(const FString address)
{
	UE_LOG(LogNotice, Display, TEXT("Joining Game"));
	if (m_mainMenu)
	{
		m_mainMenu->Deactivate();
	}
}

void AGM_Adventure_MainMenu::HostGame(const FString map)
{
	UE_LOG(LogNotice, Display, TEXT("Hosting Game"));
	if (m_mainMenu)
	{
		m_mainMenu->Deactivate();
	}

	//Start Host functions
	//UWorld* World = GetWorld();
	//if (World)
	//{
	//	World->ServerTravel("/Game/MAP_DIRECTORY_GOES_HERE?listen");
	//}
}

