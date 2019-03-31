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
#include "GameFramework/HUD.h"
#include "HUD_MPLobby.generated.h"

/**
 * 
 */
UCLASS()
class ADVENTURE_API AHUD_MPLobby : public AHUD
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, Category = "Lobby UI")
	void AddCharacter(FString Username);

	//Blueprint Functions
	UFUNCTION(BlueprintImplementableEvent, Category = "Lobby UI")
	void OnCharacterConnected(const FString& Username);

	UFUNCTION(BlueprintCallable, Category = "Lobby UI")
	void StartSession();

	UFUNCTION(BlueprintCallable, Category = "Lobby UI")
	void SetMapToLoad(const FString& Name);

	UFUNCTION(BlueprintCallable, Category = "Lobby UI")
	bool IsMapSelected()const;
	
};
