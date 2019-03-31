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

#include "DownloadManager.h"
#include "Adventure.h"

#define PACKET_SIZE 256
#define DOWNLOAD_TIME_DELAY 3.0f

TArray<uint8> ADownloadManager::m_Data = TArray<uint8>();
int ADownloadManager::m_Version = 0;

ADownloadManager::ADownloadManager()
{
	m_bPendingDownload = false;
	m_bDownloading = false;
	m_BytesDownloaded = 0;
	m_nextPacketID = 0;
	m_NotifiedVersionNo = 0;
	m_LocalVersion = 0;
	bReplicates = true;
	bAlwaysRelevant = true;
	PrimaryActorTick.bCanEverTick = true;
}

void ADownloadManager::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogNotice, Warning, TEXT("<%s>: A new download manager has been created"), *GetName());
}

void ADownloadManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority() && NewPacketAvailable())
	{
		NotifyNewDownload();
	}

	if (m_bDownloading)
	{
		SendNextPacketToClient();
	}
}

void ADownloadManager::ServerOnly_SetData(const TArray<uint8>& data)
{
	int packetCount = FMath::DivideAndRoundUp(data.Num(), PACKET_SIZE);

	m_Data = data;
	m_Version = (m_Version + 1) % MAX_int32;
	UE_LOG(LogNotice, Warning, TEXT("<DownloadManager>: New data has been made available to download: Size: %i bytes"), m_Data.Num());
}

void ADownloadManager::GetDataFromBuffer(TArray<uint8>& Data)
{
	Data = m_Data;
}

void ADownloadManager::SetOnDownloadFinishedCallback(const FNotifyDelegate & func)
{
	m_DownloadFinishedDel = func;

	if (!m_DownloadFinishedDel.IsBound())
	{
		UE_LOG(LogNotice, Error, TEXT("<%s>: Download finished callback bind fail"), *GetName());
	}
}

void ADownloadManager::SetOnPacketReceivedCallback(const FNotifyDelegate & func)
{
	m_PacketReceivedDel = func;

	if (!m_PacketReceivedDel.IsBound())
	{
		UE_LOG(LogNotice, Error, TEXT("<%s>: Packet received callback bind fail"), *GetName());
	}
}

void ADownloadManager::SetIncomingDataInfo(const FPacketInfo & info)
{
	CleanUp();
	m_DownloadInfo = info;

	// Resize the data buffer on the client to be able to hold the incoming data
	m_Data.Empty();
	m_Data.AddUninitialized(m_DownloadInfo.Size);

	UE_LOG(LogNotice, Warning, TEXT("<%s>: A new file has been made available on the server. Size: %i, Final Size: %i"), *GetName(), info.Size, m_Data.Num());
}

bool ADownloadManager::FinalizeDownload()
{
	if (m_Data.Num() != m_BytesDownloaded)
		return false;

	int BufferIndex = 0;
	int packetCount = FMath::DivideAndRoundUp(m_Data.Num(), PACKET_SIZE);
	for (int index = 0; index < packetCount; index++)
	{
		auto found = m_DownloadMap.find(index);
		if (found != m_DownloadMap.end())
		{
			for (int count = 0; count < found->second.Num(); count++)
			{
				m_Data[BufferIndex++] = found->second[count];
			}
		}
		else
		{
			m_Data.Empty();
			return false;
		}
	}

	return true;
}

bool ADownloadManager::NewPacketAvailable() const
{
	return m_LocalVersion != m_Version;
}

bool ADownloadManager::FinishedDownloading() const
{
	UE_LOG(LogNotice, Warning, TEXT("<%s>: Downloaded: %i, Total Size: %i"), *GetName(), m_BytesDownloaded, m_Data.Num());
	return (m_Data.Num() == m_BytesDownloaded);
}

float ADownloadManager::GetDataIntegrityPercentage() const
{
	return m_Data.Num() ? ((float)m_BytesDownloaded / m_Data.Num()) * 100.0f : 0;
}

FPacketInfo ADownloadManager::GetPacketInfo() const
{
	FPacketInfo newInfo;
	newInfo.Size = m_Data.Num();
	return newInfo;
}

int ADownloadManager::GetVersion() const
{
	return m_Version;
}

void ADownloadManager::CleanUp()
{
	m_Data.Empty();
	m_DownloadMap.clear();
	m_BytesDownloaded = 0;
	m_nextPacketID = 0;
}

bool ADownloadManager::GetNextPacket(TArray<uint8>& Data, int& packetNum)
{
	Data.Empty();

	// Get the array index of the next packet
	int dataIndex = m_nextPacketID * PACKET_SIZE;

	// How many bytes are left that have not been copied over
	int sendAmnt = 0;
	int remain = m_Data.Num() - dataIndex;

	if (remain > 0)
	{
		// Get bytes needed to transfer
		sendAmnt = (remain > PACKET_SIZE) ? PACKET_SIZE : remain;

		// Reserve memory to hold send amount
		Data.AddUninitialized(sendAmnt);

		// Copy memory from save binary to transfer data array
		FMemory::Memcpy(Data.GetData(), m_Data.GetData() + dataIndex, sendAmnt);

		// Set the packet num as the current next
		packetNum = m_nextPacketID++;

		return true;
	}

	return false;
}

bool ADownloadManager::AddPacket(const TArray<uint8>& Data, int packetNum)
{
	// Get the array index of the next packet
	int dataIndex = m_nextPacketID * PACKET_SIZE;

	if (dataIndex > GetPacketInfo().Size)
	{
		UE_LOG(LogNotice, Error, TEXT("<%s>:Byte buffer overload! Canceling download..."), *GetName());
		return false;
	}

	m_BytesDownloaded += Data.Num();
	m_DownloadMap[packetNum] = Data;

	return true;
}

void ADownloadManager::NotifyNewDownload()
{
	if (m_NotifiedVersionNo != m_Version)
	{
		m_NotifiedVersionNo = m_Version;
		m_DownloadInfo.Size = m_Data.Num();

		// Notify the client that new data is available
		Client_NotifyNewPacketAvailable(GetPacketInfo());
	}
}

void ADownloadManager::StartDownload()
{
	m_bDownloading = true;
	m_LocalVersion = m_Version;
}

void ADownloadManager::SendNextPacketToClient()
{
	int packetNum = 0;
	TArray<uint8> SendData;
	if (GetNextPacket(SendData, packetNum))
	{
		UE_LOG(LogNotice, Warning, TEXT("<%s>: Sending packet to client. ID: %i"), *GetName(), packetNum);
		Client_PostPacket(SendData, packetNum);
	}
	else
	{
		// Downloading has finished
		UE_LOG(LogNotice, Warning, TEXT("<%s>: Download has finished"), *GetName());
		m_bDownloading = false;
	}
}

void ADownloadManager::Client_NotifyNewPacketAvailable_Implementation(const FPacketInfo & info)
{
	// Dont run on host
	if (!GetWorld()->IsServer())
	{
		SetIncomingDataInfo(info);
		Server_StartDownload();
	}
	else
	{
		UE_LOG(LogNotice, Warning, TEXT("<%s>: File exists locally, Download Finished!"), *GetName());
		m_DownloadFinishedDel.ExecuteIfBound();
	}
}

void ADownloadManager::Client_PostPacket_Implementation(const TArray<uint8>& data, int packetNum)
{
	if (AddPacket(data, packetNum))
	{
		m_PacketReceivedDel.ExecuteIfBound();
		UE_LOG(LogNotice, Warning, TEXT("<%s>: Recieved packet #%i, Byte count: %i. Downloading %f%%"), *GetName(), packetNum, data.Num(), GetDataIntegrityPercentage());

		if (FinishedDownloading())
		{
			if (FinalizeDownload())
			{
				UE_LOG(LogNotice, Warning, TEXT("<%s>: Download Success!"));
				m_DownloadFinishedDel.ExecuteIfBound();
			}
			else
			{
				UE_LOG(LogNotice, Error, TEXT("<%s>: Download failed to finalize!"), *GetName());
			}
		}
	}
	else
	{
		UE_LOG(LogNotice, Warning, TEXT("<%s>: Recieved packet but was unable to add it. PacketNum: %i"), *GetName(), packetNum);
	}
}

void ADownloadManager::Server_StartDownload_Implementation()
{
	UE_LOG(LogNotice, Warning, TEXT("<%s>: Client wants to start downloading..."), *GetName());
	if(NewPacketAvailable())
		GetWorld()->GetTimerManager().SetTimer(m_BeginDownloadTimer, this, &ADownloadManager::StartDownload, 1.0f, false, DOWNLOAD_TIME_DELAY);
	else
		UE_LOG(LogNotice, Warning, TEXT("<%s>: Download canceled, no data exists to be sent"), *GetName());
}

bool ADownloadManager::Server_StartDownload_Validate()
{
	return true;
}