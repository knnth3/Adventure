// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "DownloadManager.h"
#include "Adventure.h"

#define PACKET_SIZE (int32)sizeof(FVector)
#define PACKET_TRANSFER_TIME_DELAY 3.0f

TArray<uint8> APacketManager::m_Data = TArray<uint8>();
int APacketManager::m_Version = 0;

APacketManager::APacketManager()
{
	m_ElapsedTime = 0;
	m_DownloadedSize = 0;
	bReplicates = false;
	m_bDownloading = false;
	m_bReadyToDownload = false;
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

	if (m_bDownloading)
	{
		if (HasAuthority())
		{
			Client_Ping(FVector::ZeroVector);
		}
	}
}

void APacketManager::ServerOnly_SetData(const TArray<uint8>& data)
{
	int packetCount = FMath::DivideAndRoundUp(data.Num(), PACKET_SIZE);

	// File is too large to send over
	if (packetCount > TRANSFER_BITFIELD_SIZE)
	{
		UE_LOG(LogNotice, Error, TEXT("<PacketManager>: Could not set data for download: File is too large."));
		return;
	}

	m_Data = data;
	m_Version = (m_Version + 1) % MAX_int32;
	UE_LOG(LogNotice, Warning, TEXT("<PacketManager>: New data has been maid available to download: Size: %i bytes"), m_Data.Num());
}

void APacketManager::BeginDownload()
{
	if (m_bReadyToDownload)
	{
		m_bDownloading = true;
		m_bReadyToDownload = false;
	}
}

void APacketManager::GetDataFromBuffer(TArray<uint8>& Data)
{
	Data = m_Data;
}

void APacketManager::SetOnDataPostedCallback(const FNotifyDelegate & func)
{
	UE_LOG(LogNotice, Warning, TEXT("<%s>: Successfully bound callback"), *GetName());
	m_NotifyFunc = func;

	// Check to see if data was posted before a callback was set
	if (m_bReadyToDownload)
	{
		// Notify the client that new data is available
		m_NotifyFunc.ExecuteIfBound();
	}
}

bool APacketManager::IsDownloading() const
{
	return m_bDownloading;
}

float APacketManager::GetDataIntegrityPercentage() const
{
	return (float)m_DownloadedSize / m_Data.Num();
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
}

void APacketManager::Client_Ping_Implementation(const FVector & loc)
{
	UE_LOG(LogNotice, Warning, TEXT("<%s>: Ping!"), *GetName());
}

void APacketManager::GetSendPacket(TArray<uint8>& OutData, TArray<int32>& NextBit)
{
	bool bLastPacket = false;
	TArray<uint8> sendingData;
	auto nextBit = GetNextPacketData(sendingData);

	// Send the new data to the client (if any exists)
	if (sendingData.Num())
	{
		OutData = sendingData;
		NextBit = BitsetToArray<TRANSFER_BITFIELD_SIZE>(nextBit);
		m_Bitfield |= nextBit;
		UE_LOG(LogNotice, Warning, TEXT("<%s>: Net connection sent packet"), *GetName());
	}
}

void APacketManager::AddPacket()
{
}

void APacketManager::OnNewDataPosted()
{
	UE_LOG(LogNotice, Warning, TEXT("<%s>: A new file has been made available on the server"), *GetName());

	m_Bitfield = 0;
	m_DownloadedSize = 0;
	m_bReadyToDownload = true;

	// Resize the data buffer on the client to be able to hold the incoming data
	m_Data.Empty();
	m_Data.AddUninitialized(m_DownloadInfo.Size);

	// Notify the client that new data is available
	m_NotifyFunc.ExecuteIfBound();
}

std::bitset<TRANSFER_BITFIELD_SIZE> APacketManager::GetNextPacketData(TArray<uint8>& Data)
{
	Data.Empty();

	// Get the next bit to represent outgoing package (assumes all bits before it are on)
	int dataIndex = 0;
	std::bitset<TRANSFER_BITFIELD_SIZE> NextBit = 0;
	for (int index = 0; index < m_Bitfield.size(); index++)
	{
		if (m_Bitfield[index] == 0)
		{
			NextBit[index] = 1;
			dataIndex = index * PACKET_SIZE;
			break;
		}
	}

	if (dataIndex == 0 && NextBit.none())
	{
		UE_LOG(LogNotice, Error, TEXT("<%s>:Byte buffer overload! Canceling download..."), *GetName());
		return NextBit;
	}

	// How many bytes are left that have not been copied over
	int sendAmnt = 0;
	int remain = m_Data.Num() - dataIndex;
	if (remain > 0)
	{
		// Get bytes needed to transfer
		sendAmnt = (remain > PACKET_SIZE) ? PACKET_SIZE : remain;

		// Last packet is being sent
		if (sendAmnt == remain)
		{
			m_bDownloading = false;
		}

		// Reserve memory to hold send amount
		Data.AddUninitialized(sendAmnt);

		// Copy memory from save binary to transfer data array
		FMemory::Memcpy(Data.GetData(), m_Data.GetData() + dataIndex, sendAmnt);
	}

	return NextBit;
}

void APacketManager::Client_PostNewPacket_Implementation(const TArray<uint8>& Data, const TArray<int>& Bitfield)
{
	// Reset timer that tracks whether the last packet was sent
	m_ElapsedTime = 0;

	// If there are changes to be made
	auto RecievedPacketBit = ArrayToBitset<TRANSFER_BITFIELD_SIZE>(Bitfield);

	if (!(m_Bitfield^RecievedPacketBit).none())
	{
		auto BFcurrentPacket = RecievedPacketBit;

		// Get the position in the buffer where the new data should go
		int CurrentIndex = 0;
		for (int x = 0; x < TRANSFER_BITFIELD_SIZE; x++)
		{
			if (BFcurrentPacket[x] == true)
			{
				CurrentIndex = x * PACKET_SIZE;
				break;
			}
		}

		m_Bitfield |= RecievedPacketBit;
		m_DownloadedSize += Data.Num();
		FString BitsetStr(m_Bitfield.to_string().c_str());
		UE_LOG(LogNotice, Warning, TEXT("<%s>: Downloading (%i/%i): Bitfield: %s"), *GetName(), m_DownloadedSize, m_Data.Num(), *BitsetStr);

		// Transfer the nessesary data to the correct location in the buffer
		FMemory::Memcpy(m_Data.GetData() + CurrentIndex, Data.GetData(), Data.Num());

		if (m_DownloadedSize == m_Data.Num())
		{
			UE_LOG(LogNotice, Warning, TEXT("<%s>: Map download complete!"), *GetName());
			m_bDownloading = false;
			m_bReadyToDownload = false;
		}

	}
	else
	{
		UE_LOG(LogNotice, Warning, TEXT("<%s>: A packet was ignored, data already sent!"), *GetName());
	}
}

void APacketManager::Client_PostLastNewPacket_Implementation(const TArray<uint8>& Data, const TArray<int>& Bitfield)
{
	Client_PostNewPacket(Data, Bitfield);
}

void APacketManager::Server_RequestPacket_Implementation(const TArray<int>& BFRecieved)
{
	UE_LOG(LogNotice, Warning, TEXT("<%s>: Client request to download acknowldged"), *GetName());

	// Start the downloading process using the given bitfield
	m_Bitfield = ArrayToBitset<TRANSFER_BITFIELD_SIZE>(BFRecieved);
	m_bDownloading = true;
}

bool APacketManager::Server_RequestPacket_Validate(const TArray<int>& BFRecieved)
{
	return true;
}

void APacketManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APacketManager, m_DownloadInfo);
}