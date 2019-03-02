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
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Setup PacketManager on new connections
	void SetupPacketManager();

	//Sets player ID (Server)
	void SetPlayerID(const int ID);

	// Gets player ID
	int GetPlayerID()const;

	// Exec function for debugging purposes
	UFUNCTION(Exec, Category = ExecFunctions)
	void ShowPathfindingDebugLines(bool Value);

private:
	UFUNCTION()
	void GenerateGrid();
	
	UFUNCTION()
	void SetupForDownloading();

	UFUNCTION()
	void CreatePacketManager();

	UFUNCTION()
	void OnDataSetNotify();

	// Receive packet from server
	UFUNCTION(Client, Reliable)
	void Client_NotifyNewPacketAvailable(const FPacketInfo& info);

	// Receive packet from server
	UFUNCTION(Client, Reliable)
	void Client_PostPacket(const TArray<uint8>& data, int packetNum);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_StartDownload();
	
	UPROPERTY()
	int UniqueID;

	UPROPERTY()
	FPacketInfo m_DLPacketInfo;

	float m_ElapsedTime;
	bool m_bClientPendingDownloadStart;
	bool m_bServerStartTiming;
	bool m_bServerStartDownload;
	APacketManager* m_PacketManager;
};
