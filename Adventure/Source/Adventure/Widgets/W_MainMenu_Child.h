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

#pragma once

#include "CoreMinimal.h"
#include "UI_MainMenu.h"
#include "Blueprint/UserWidget.h"
#include "W_MainMenu_Child.generated.h"

/**
 * 
 */
UCLASS()
class ADVENTURE_API UW_MainMenu_Child : public UUserWidget
{
	GENERATED_BODY()
	
public:

	void ConnectTo(class UW_MainMenu* MainMenu);

protected:

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void RequestHostGame(FHOSTGAME_SETTINGS settings);

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void RequestJoinGame(FJOINGAME_SETTINGS settings);

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void RequestLaunchGameBuilder(FGAMEBUILDER_SETTINGS settings);

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void GetServerList(TArray<FString>& Array)const;

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	virtual bool IsSessionSearchActive()const;

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	bool GetAllSaveGameSlotNames(TArray<FString>& Array, FString Ext);

private:

	class UW_MainMenu* MainMenu = nullptr;
	
};
