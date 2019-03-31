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
#include "GI_Adventure.h"
#include "Blueprint/UserWidget.h"
#include "W_PauseMenu.generated.h"

/**
 * 
 */
UCLASS()
class ADVENTURE_API UW_PauseMenu : public UUserWidget
{
	GENERATED_BODY()
	
protected:

	virtual bool Initialize()override;
	
public:

	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	void Activate();

	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	void Deactivate();

	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	void ExitToMainMenu();

	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	bool IsActive()const;


private:

	bool bIsActive;
	class UGI_Adventure* InstanceInterface;
	
};
