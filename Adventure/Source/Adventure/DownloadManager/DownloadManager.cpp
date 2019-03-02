// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "DownloadManager.h"
#include "Adventure.h"

#define PACKET_SIZE 256

TArray<uint8> APacketManager::m_Data = TArray<uint8>();
int APacketManager::m_Version = 0;

APacketManager::APacketManager()
{
	m_BytesDownloaded = 0;
	m_nextPacketID = 0;
	m_NotifiedVersionNo = 0;
	m_LocalVersion = 0;
	bReplicates = false;
	PrimaryActorTick.bCanEverTick = true;
}

void APacketManager::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogNotice, Warning, TEXT("<%s>: A new download manager has been created"), *GetName());
}

void APacketManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority() && NewPacketAvailable())
	{
		NotifyNewDownload();
	}
}

void APacketManager::ServerOnly_SetData(const TArray<uint8>& data)
{
	int packetCount = FMath::DivideAndRoundUp(data.Num(), PACKET_SIZE);

	m_Data = data;
	m_Version = (m_Version + 1) % MAX_int32;
	UE_LOG(LogNotice, Warning, TEXT("<PacketManager>: New data has been made available to download: Size: %i bytes"), m_Data.Num());
}

bool APacketManager::BeginDownload()
{
	if (NewPacketAvailable())
	{
		m_LocalVersion = m_Version;

		return true;
	}

	return false;
}

void APacketManager::GetDataFromBuffer(TArray<uint8>& Data)
{
	Data = m_Data;
}

void APacketManager::SetOnDataPostedCallback(const FNotifyDelegate & func)
{
	UE_LOG(LogNotice, Warning, TEXT("<%s>: Current Version: %i, Local Version: %i, Notified Version: %i"), *GetName(), m_Version, m_LocalVersion, m_NotifiedVersionNo);
	m_NotifyFunc = func;

	if (m_NotifyFunc.IsBound())
	{
		UE_LOG(LogNotice, Warning, TEXT("<%s>: Callback bind success"), *GetName());
	}
	else
	{
		UE_LOG(LogNotice, Warning, TEXT("<%s>: Callback bind fail"), *GetName());
	}
}

void APacketManager::SetIncomingDataInfo(const FPacketInfo & info)
{
	CleanUp();
	m_DownloadInfo = info;

	// Resize the data buffer on the client to be able to hold the incoming data
	m_Data.Empty();
	m_Data.AddUninitialized(m_DownloadInfo.Size);

	UE_LOG(LogNotice, Warning, TEXT("<%s>: A new file has been made available on the server. Size: %i, Final Size: %i"), *GetName(), info.Size, m_Data.Num());
}

bool APacketManager::FinalizeDownload()
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

bool APacketManager::NewPacketAvailable() const
{
	return m_LocalVersion != m_Version;
}

bool APacketManager::FinishedDownloading() const
{
	UE_LOG(LogNotice, Warning, TEXT("<%s>: Downloaded: %i, Total Size: %i"), *GetName(), m_BytesDownloaded, m_Data.Num());
	return (m_Data.Num() == m_BytesDownloaded);
}

float APacketManager::GetDataIntegrityPercentage() const
{
	return m_Data.Num() ? ((float)m_BytesDownloaded / m_Data.Num()) * 100.0f : 0;
}

FPacketInfo APacketManager::GetPacketInfo() const
{
	FPacketInfo newInfo;
	newInfo.Size = m_Data.Num();
	return newInfo;
}

int APacketManager::GetVersion() const
{
	return m_Version;
}

void APacketManager::CleanUp()
{
	m_Data.Empty();
	m_DownloadMap.clear();
	m_BytesDownloaded = 0;
	m_nextPacketID = 0;
}

bool APacketManager::GetNextPacket(TArray<uint8>& Data, int& packetNum)
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

bool APacketManager::AddPacket(const TArray<uint8>& Data, int packetNum)
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

	//// Reset timer that tracks whether the last packet was sent
	//m_ElapsedTime = 0;

	//// If there are changes to be made
	//auto RecievedPacketBit = ArrayToBitset<TRANSFER_BITFIELD_SIZE>(Bitfield);

	//if (!(m_Bitfield^RecievedPacketBit).none())
	//{
	//	auto BFcurrentPacket = RecievedPacketBit;

	//	// Get the position in the buffer where the new data should go
	//	int CurrentIndex = 0;
	//	for (int x = 0; x < TRANSFER_BITFIELD_SIZE; x++)
	//	{
	//		if (BFcurrentPacket[x] == true)
	//		{
	//			CurrentIndex = x * PACKET_SIZE;
	//			break;
	//		}
	//	}

	//	m_Bitfield |= RecievedPacketBit;
	//	m_DownloadedSize += Data.Num();
	//	FString BitsetStr(m_Bitfield.to_string().c_str());
	//	UE_LOG(LogNotice, Warning, TEXT("<%s>: Downloading (%i/%i): Bitfield: %s"), *GetName(), m_DownloadedSize, m_Data.Num(), *BitsetStr);

	//	// Transfer the nessesary data to the correct location in the buffer
	//	FMemory::Memcpy(m_Data.GetData() + CurrentIndex, Data.GetData(), Data.Num());

	//	if (m_DownloadedSize == m_Data.Num())
	//	{
	//		UE_LOG(LogNotice, Warning, TEXT("<%s>: Map download complete!"), *GetName());
	//		m_bDownloading = false;
	//		m_bReadyToDownload = false;
	//	}

	//}
	//else
	//{
	//	UE_LOG(LogNotice, Warning, TEXT("<%s>: A packet was ignored, data already sent!"), *GetName());
	//}
}

void APacketManager::NotifyNewDownload()
{
	if (m_NotifiedVersionNo != m_Version)
	{
		m_NotifiedVersionNo = m_Version;
		m_DownloadInfo.Size = m_Data.Num();

		UE_LOG(LogNotice, Warning, TEXT("<%s>: Calling notify..."), *GetName());

		// Notify the client that new data is available
		m_NotifyFunc.ExecuteIfBound();
	}
}