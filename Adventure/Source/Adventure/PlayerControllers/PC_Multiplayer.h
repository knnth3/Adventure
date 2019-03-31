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

#include <vector>
#include "CoreMinimal.h"
#include "DownloadManager/DownloadManager.h"
#include "GameFramework/PlayerController.h"
#include "PC_Multiplayer.generated.h"


UCLASS()
class ADVENTURE_API APC_Multiplayer : public APlayerController
{
	GENERATED_BODY()
public:
	APC_Multiplayer();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	//Sets player ID (Server)
	void SetPlayerID(const int ID);

	// Gets player ID
	int GetPlayerID()const;

	// Exec function for debugging purposes
	UFUNCTION(Exec, Category = ExecFunctions)
	void ShowPathfindingDebugLines(bool Value);

	UFUNCTION(BlueprintCallable, Category = "Download Manager")
	float GetDownloadPercentage() const;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Download Manager")
	void OnDownloadCompleteEvent();

	UFUNCTION(BlueprintImplementableEvent, Category = "Download Manager")
	void OnPacketReceivedEvent();

private:
	UFUNCTION()
	void OnPacketReceived();

	UFUNCTION()
	void OnDownloadComplete();

	UFUNCTION()
	void GenerateGrid();

	UFUNCTION()
	void OnNewDownloadManager();
	
	UPROPERTY()
	int UniqueID;

	UPROPERTY(ReplicatedUsing = OnNewDownloadManager)
	ADownloadManager* m_DownloadManager;
};
