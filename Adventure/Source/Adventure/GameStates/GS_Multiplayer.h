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

#include <map>
#include <deque>
#include "Grid/WorldGrid.h"
#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GS_Multiplayer.generated.h"

/**
 * 
 */

UCLASS()
class ADVENTURE_API AGS_Multiplayer : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	AGS_Multiplayer();
	virtual void HandleBeginPlay()override;
	void AddNewPlayer(int PlayerID, FString PlayerName);
	void GenerateGrid();

	UFUNCTION(BlueprintCallable, Category = "Turn-Based Mechanics")
	void SetActivePlayer(const int ID);

	UFUNCTION(BlueprintCallable, Category = "Turn-Based Mechanics")
	FString GetActivePlayerName() const;

	UFUNCTION(BlueprintCallable, Category = "Turn-Based Mechanics")
	FString GetPlayerName(int PlayerID) const;

	UFUNCTION(BlueprintCallable, Category = "Turn-Based Mechanics")
	TArray<FString> GetAllPlayerNames() const;

	UFUNCTION(BlueprintCallable, Category = "Turn-Based Mechanics")
	int GetPlayerID(FString PlayerName)const;

protected:

	UPROPERTY(EditAnywhere, Category = "Spawnables")
	TSubclassOf<class AWorldGrid> GridClass;

private:

	UPROPERTY(Replicated)
	FString m_ActivePlayerName;

	UPROPERTY(Replicated)
	TArray<FString> m_PlayerNameArray;

	bool m_bFreeRoamActive;
	int m_CurrentActivePlayer;
	class AWorldGrid* m_WorldGrid;
	std::deque<int> m_TurnSequence;
};
