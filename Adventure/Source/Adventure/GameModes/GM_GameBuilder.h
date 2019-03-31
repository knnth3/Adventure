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

#include "Basics.h"
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GM_GameBuilder.generated.h"

/**
 * 
 */
UCLASS()
class ADVENTURE_API AGM_GameBuilder : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	UFUNCTION(BlueprintCallable, Category = "GameBuilder")
	bool IsNewMap()const;

	UFUNCTION(BlueprintCallable, Category = "GameBuilder")
	FGridCoordinate GetMapSize()const;

	UFUNCTION(BlueprintCallable, Category = "GameBuilder")
	FString GetMapName()const;

private:

	bool m_bNewMap;
	FString m_MapName;
	FGridCoordinate m_GridDimensions;
};
