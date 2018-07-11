// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.
#include "W_PauseMenu.h"
#include "Adventure.h"

bool UW_PauseMenu::Initialize()
{
	bool preInit = Super::Initialize();
	if (preInit)
	{
		bIsActive = false;
		InstanceInterface = nullptr;
	}

	return preInit;
}

void UW_PauseMenu::Activate()
{
	AddToViewport();
	bIsActive = true;
	UE_LOG(LogNotice, Warning, TEXT("Paused"));
}

void UW_PauseMenu::Deactivate()
{
	RemoveFromViewport();
	bIsActive = false;
	UE_LOG(LogNotice, Warning, TEXT("Unpaused"));
}

void UW_PauseMenu::ExitToMainMenu()
{
	Deactivate();

	UWorld* World = GetWorld();
	if (World)
	{
		UGI_Adventure* GameInstance = Cast<UGI_Adventure>(World->GetGameInstance());
		if (GameInstance)
		{
			GameInstance->Disconnect();
		}
	}
}

bool UW_PauseMenu::IsActive() const
{
	return bIsActive;
}
