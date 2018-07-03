// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "W_Lobby.h"



bool UW_Lobby::Activate()
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

void UW_Lobby::Deactivate()
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

void UW_Lobby::AddCharacter(FString Username)
{
	OnCharacterConnected(Username);
}