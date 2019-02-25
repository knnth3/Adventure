// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

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
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Initializes the network manager and notifies all incoming connections about changes
	void InitNetworkManager();

	//Sets player ID (Server)
	void SetPlayerID(const int ID);

	// Gets player ID
	int GetPlayerID()const;

	// Exec function for debugging purposes
	UFUNCTION(Exec, Category = ExecFunctions)
	void ShowPathfindingDebugLines(bool Value);

private:

	UFUNCTION()
	void OnDownloadManagerCreated();

	UFUNCTION()
	void OnNewDataAvailable();
	
	UPROPERTY()
	int UniqueID;

	UPROPERTY(Replicated = OnDownloadManagerCreated)
	ADownloadManager* m_DownloadManager;

	float m_ElapsedTime;
	bool m_bNewDownloadAvailable;
};
