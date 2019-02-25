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

#define TRANSFER_BITFIELD_SIZE sizeof(int) * 8 * 5
DECLARE_DELEGATE(FNotifyDelegate);

USTRUCT()
struct FDownloadInfo
{
	GENERATED_BODY()
public:

	UPROPERTY()
	int PackageSize;
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

	UFUNCTION(BlueprintCallable, Category = "Download Manager")
	void BeginDownload();

	UFUNCTION(BlueprintCallable, Category = "Download Manager")
	void GetDataFromBuffer(TArray<uint8>& Data);

	void SetOnDataPostedCallback(const FNotifyDelegate& func);

	void CleanUp();

	// Receive packet from server
	UFUNCTION(Client, Unreliable)
	void Client_Ping(const FVector& loc);

private:

	// Client function call to tell the server it wants the data
	void RequestPacket();

	// Server function call to send next packet to client
	void SendPacket(float DeltaTime);

	// Notifies clients that new data is available
	void NotifyDataChanged();

	UFUNCTION()
	void OnNewDataPosted();

	// Get raw data at m_NextPacket (TRANSFER_DATA_SIZE interval)
	std::bitset<TRANSFER_BITFIELD_SIZE> GetNextPacketData(TArray<uint8>& Data);

	// Receive packet from server
	UFUNCTION(Client, Unreliable)
	void Client_PostNewPacket(const TArray<uint8>& Data, const TArray<int>& Bitfield);

	// Receive packet from server
	UFUNCTION(Client, Reliable)
	void Client_PostLastNewPacket(const TArray<uint8>& Data, const TArray<int>& Bitfield);

	// Request a packet from the server
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_RequestPacket(const TArray<int>& BFRecieved);

	UPROPERTY(ReplicatedUsing = OnNewDataPosted)
	FDownloadInfo m_DownloadInfo;

	FNotifyDelegate m_NotifyFunc;
	float m_ElapsedTime;
	bool m_bReadyToDownload;
	bool m_bDownloading;
	int m_DownloadedSize;
	int m_localVer;
	static int m_GlobalVer;
	static TArray<uint8> m_Data;
	FSocket* m_ConnectionSocket;
	FIPv4Endpoint m_RemoteAddr;
	std::bitset<TRANSFER_BITFIELD_SIZE> m_Bitfield;
};