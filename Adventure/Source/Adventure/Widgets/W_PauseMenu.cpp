// Copyright 2019 Eric Marquez
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
// http ://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
