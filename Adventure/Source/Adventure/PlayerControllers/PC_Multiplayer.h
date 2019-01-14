// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PC_Multiplayer.generated.h"

/**
 * 
 */
UCLASS()
class ADVENTURE_API APC_Multiplayer : public APlayerController
{
	GENERATED_BODY()
public:
	APC_Multiplayer();
	void SetPlayerID(const int ID);
	int GetPlayerID()const;
	void ShouldDownloadMap(bool bHasMap);

	UFUNCTION(Exec, Category = ExecFunctions)
	void ShowPathfindingDebugLines(bool Value);

protected:

	// Tick function
	virtual void Tick(float DeltaTime) override;
	int GetNextPacketData();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_DownloadMap(int packetID);

	UFUNCTION(Client, Reliable)
	void Client_RecievePacket(int Data, bool LastPacket);

private:
	
	UPROPERTY()
	int UniqueID;
	
	// Client
	int m_CurrentDownloadPacketID;
	bool m_bMapDownloaded;

	// Server
	int m_NextPacket;
	bool m_bNeedsNextPacket;
	float m_TotalTime;
};
