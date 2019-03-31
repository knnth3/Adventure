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

	void SetOnPacketReceivedCallback(const FNotifyDelegate& func);

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
	FNotifyDelegate m_PacketReceivedDel;
	FNotifyDelegate m_DownloadFinishedDel;
	std::map<int, TArray<uint8>> m_DownloadMap;
};