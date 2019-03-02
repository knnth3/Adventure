// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include <memory>
#include <map>
#include "Networking.h"
#include "Basics.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DownloadManager.generated.h"

/**
 * 
 */

DECLARE_DELEGATE(FNotifyDelegate);

USTRUCT()
struct FPacketInfo
{
	GENERATED_BODY()
public:

	UPROPERTY()
	int Size;
};

UCLASS()
class ADVENTURE_API ADownloadManager : public AActor
{
	GENERATED_BODY()
	
public:
	ADownloadManager();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	static void ServerOnly_SetData(const TArray<uint8>& data);

	UFUNCTION(BlueprintCallable, Category = "Packet Manager")
	void GetDataFromBuffer(TArray<uint8>& Data);

	void SetOnDownloadFinishedCallback(const FNotifyDelegate& func);

	bool NewPacketAvailable()const;

	bool FinishedDownloading() const;

	float GetDataIntegrityPercentage() const;

	FPacketInfo GetPacketInfo()const;

	int GetVersion()const;

	void CleanUp();

private:

	// Server function call to send next packet to client
	bool GetNextPacket(TArray<uint8>& OutData, int& packetNum);

	// Client function to add incoming packet
	bool AddPacket(const TArray<uint8>& Data, int packetNum);

	bool FinalizeDownload();
	void SetIncomingDataInfo(const FPacketInfo& info);
	void NotifyNewDownload();
	void StartDownload();
	void SendNextPacketToClient();

	// Receive packet from server
	UFUNCTION(Client, Reliable)
	void Client_NotifyNewPacketAvailable(const FPacketInfo& info);

	// Receive packet from server
	UFUNCTION(Client, Reliable)
	void Client_PostPacket(const TArray<uint8>& data, int packetNum);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_StartDownload();

	static int m_Version;
	static TArray<uint8> m_Data;
	bool m_bPendingDownload;
	bool m_bDownloading;
	int m_BytesDownloaded;
	int m_nextPacketID;
	int m_NotifiedVersionNo;
	int m_LocalVersion;
	FPacketInfo m_DownloadInfo;
	FTimerHandle m_BeginDownloadTimer;
	FNotifyDelegate m_DownloadFinishedDel;
	std::map<int, TArray<uint8>> m_DownloadMap;
};