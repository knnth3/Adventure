// Fill out your copyright Warning in the Description page of Project Settings.

#include "W_MainMenu.h"
#include "Adventure.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "W_MainMenu_Child.h"


bool UW_MainMenu::Initialize()
{
	bool preInit = Super::Initialize();
	if (!preInit)
	{
		UE_LOG(LogNotice, Error, TEXT("Failed to create menu: %s"), *this->GetName());
	}
	if (Play)
	{
		Play->OnClicked.AddDynamic(this, &UW_MainMenu::LoadNextState);
	}

	m_active = ACTIVE_MENU::HOME;
	return preInit;
}

void UW_MainMenu::LoadNextState()
{
	switch (m_active)
	{
	case ACTIVE_MENU::HOME:
		UE_LOG(LogNotice, Error, TEXT("Load Next state was Home"));
		break;
	case ACTIVE_MENU::JOIN:
		JoinGame("");
		break;
	case ACTIVE_MENU::HOST:
		HostGame("");
		break;
	case ACTIVE_MENU::GAMEBUILDER:
		UE_LOG(LogNotice, Error, TEXT("Load Next state was GameBuilder"));
		break;
	case ACTIVE_MENU::CUSTOM:
		break;
	default:
		break;
	}
}

void UW_MainMenu::HostGame(const FString mapName)
{
	if (m_interface)
	{
		m_interface->HostGame(mapName);
	}
}

void UW_MainMenu::JoinGame(const FString address)
{
	if (m_interface)
	{
		m_interface->JoinGame(address);
	}
}

void UW_MainMenu::LoadEditor(const FString mapName)
{
}

void UW_MainMenu::ShowHostMenu()
{
	if (SubMenu && m_hostMenu)
	{
		SubMenu->SetActiveWidget(m_hostMenu);
		m_active = ACTIVE_MENU::HOST;
	}
}

void UW_MainMenu::ShowJoinMenu()
{
	if (SubMenu && m_joinMenu)
	{
		SubMenu->SetActiveWidget(m_joinMenu);
		m_active = ACTIVE_MENU::JOIN;
	}
}

void UW_MainMenu::ShowGameBuilderMenu()
{
	if (SubMenu && m_gameBuilderMenu)
	{
		SubMenu->SetActiveWidget(m_gameBuilderMenu);
		m_active = ACTIVE_MENU::GAMEBUILDER;
	}
}

void UW_MainMenu::SetHostGameMenuClass(TSubclassOf<UW_MainMenu_Child> HostMenuClass)
{
	if (SubMenu && HostMenuClass)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			m_hostMenu = CreateWidget<UW_MainMenu_Child>(World, HostMenuClass);
			if (m_hostMenu)
			{
				m_hostMenu->ConnectTo(this);
				SubMenu->AddChild(m_hostMenu);
			}
		}
	}
}

void UW_MainMenu::SetJoinGameMenuClass(TSubclassOf<UW_MainMenu_Child> JoinMenuClass)
{
	if (SubMenu && JoinMenuClass)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			m_joinMenu = CreateWidget<UW_MainMenu_Child>(World, JoinMenuClass);
			if (m_joinMenu)
			{
				m_joinMenu->ConnectTo(this);
				SubMenu->AddChild(m_joinMenu);
			}
		}
	}
}

void UW_MainMenu::SetGameBuilderMenuClass(TSubclassOf<UW_MainMenu_Child> GameBuilderMenuClass)
{
	if (SubMenu && GameBuilderMenuClass)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			m_gameBuilderMenu = CreateWidget<UW_MainMenu_Child>(World, GameBuilderMenuClass);
			if (m_gameBuilderMenu)
			{
				m_gameBuilderMenu->ConnectTo(this);
				SubMenu->AddChild(m_gameBuilderMenu);
			}
		}
	}
}

void UW_MainMenu::AddCallbackInterface(IUI_MainMenu* Interface)
{
	m_interface = Interface;
}

bool UW_MainMenu::Activate()
{
	bool success = false;
	AddToViewport();

	UWorld* World = GetWorld();
	if (World)
	{
		//Get player controller to set Input Mode
		APlayerController* Controller = World->GetFirstPlayerController();
		if (Controller)
		{
			//Set Focus
			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

			Controller->SetInputMode(InputMode);

			success = true;
		}
	}

	return success;
}

void UW_MainMenu::Deactivate()
{
	UWorld* World = GetWorld();
	if (World)
	{
		//Get player controller to set Input Mode
		APlayerController* Controller = World->GetFirstPlayerController();
		if (Controller)
		{
			//Set Focus
			FInputModeGameOnly InputMode;
			Controller->SetInputMode(InputMode);
		}
	}

	RemoveFromViewport();
}

void UW_MainMenu::SetGameBuilderSettings(GAMEBUILDER_SETTINGS settings)
{
	m_gbSettings = settings;
}