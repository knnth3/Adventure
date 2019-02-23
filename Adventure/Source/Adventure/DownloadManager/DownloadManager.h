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

USTRUCT()
struct FDownloadInfo
{
	GENERATED_BODY()
public:

	UPROPERTY()
	int PackageSize;

	UPROPERTY()
	TArray<int> FinalizedBitField;
};

UCLASS()
class ADVENTURE_API ADownloadManager : public AActor
{
	GENERATED_BODY()
	
public:
	ADownloadManager();
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category="Data Settings")
	void ServerOnly_SetData(const TArray<uint8>& data);

	UFUNCTION(BlueprintCallable, Category = "Data Settings")
	void Subscribe(UNetConnection* connection = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Download Manager")
	void BeginDownload();

	UFUNCTION(BlueprintCallable, Category = "Download Manager")
	void GetDataFromBuffer(TArray<uint8>& Data);

private:

	FSocket* CreateClientSocket(UNetConnection* connection);
	FSocket* CreateServerSocket();
	bool FormatIP4ToNumber(const FString& TheIP, uint8(&Out)[4]);

	// Client function call to retrieve next packet from server
	void RequestPacket();

	UFUNCTION()
	void OnNewDataPosted();

	// Get raw data at m_NextPacket (TRANSFER_DATA_SIZE interval)
	std::bitset<TRANSFER_BITFIELD_SIZE> GetNextPacketData(TArray<uint8>& Data);

	// Receive packet from server
	UFUNCTION(Client, Unreliable)
	void Client_PostNewPacket(const TArray<uint8>& Data, const TArray<int>& Bitfield);

	// Request a packet from the server
	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_RequestPacket(const TArray<int>& BFRecieved);

	UPROPERTY(ReplicatedUsing = OnNewDataPosted)
	FDownloadInfo m_DownloadInfo;

	float m_ElapsedTime;
	bool m_bReadyToDownload;
	bool m_bDownloading;
	int m_DownloadedSize;
	TArray<uint8> m_Data;
	FSocket* m_ConnectionSocket;
	FIPv4Endpoint m_RemoteAddr;
	std::bitset<TRANSFER_BITFIELD_SIZE> m_Bitfield;
};