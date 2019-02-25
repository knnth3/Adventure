// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "DownloadManager.h"
#include "Adventure.h"

#define PACKET_SIZE (int32)sizeof(FVector)
#define PACKET_TRANSFER_TIME_DELAY 3.0f

TArray<uint8> ADownloadManager::m_Data = TArray<uint8>();
int ADownloadManager::m_GlobalVer = 0;

ADownloadManager::ADownloadManager()
{
	m_ElapsedTime = 0;
	bReplicates = true;
	bAlwaysRelevant = true;
	m_DownloadedSize = 0;
	m_bDownloading = false;
	m_bReadyToDownload = false;
	PrimaryActorTick.bCanEverTick = true;
	m_localVer = 0;
}

void ADownloadManager::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogNotice, Warning, TEXT("<DownloadManager>: A new download manager has been created"));
}

void ADownloadManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Check if any updates are available
	if (HasAuthority() && m_GlobalVer != m_localVer)
	{
		m_localVer = m_GlobalVer;
		NotifyDataChanged();
	}

	if (m_bDownloading)
	{
		if (HasAuthority())
		{
			Client_Ping(FVector::ZeroVector);
		}
	}
}

void ADownloadManager::ServerOnly_SetData(const TArray<uint8>& data)
{
	int packetCount = FMath::DivideAndRoundUp(data.Num(), PACKET_SIZE);

	// File is too large to send over
	if (packetCount > TRANSFER_BITFIELD_SIZE)
	{
		UE_LOG(LogNotice, Error, TEXT("<DownloadManager>: Could not set data for download: File is too large."));
		return;
	}

	m_Data = data;
	m_GlobalVer = (m_GlobalVer + 1) % MAX_int32;
	UE_LOG(LogNotice, Warning, TEXT("<DownloadManager>: Data set success"));
}

void ADownloadManager::BeginDownload()
{
	if (m_bReadyToDownload)
	{
		m_bReadyToDownload = false;
		RequestPacket();
	}
}

void ADownloadManager::GetDataFromBuffer(TArray<uint8>& Data)
{
	Data = m_Data;
}

void ADownloadManager::SetOnDataPostedCallback(const FNotifyDelegate & func)
{
	UE_LOG(LogNotice, Warning, TEXT("<DownloadManager>: Successfully bound callback"));
	m_NotifyFunc = func;

	// Check to see if data was posted before a callback was set
	if (m_bReadyToDownload)
	{
		// Notify the client that new data is available
		m_NotifyFunc.ExecuteIfBound();
	}
}

void ADownloadManager::CleanUp()
{
}

void ADownloadManager::Client_Ping_Implementation(const FVector & loc)
{
	UE_LOG(LogNotice, Warning, TEXT("<DownloadManager>: Ping!"));
}

void ADownloadManager::RequestPacket()
{
	m_bDownloading = true;
	Server_RequestPacket(BitsetToArray<TRANSFER_BITFIELD_SIZE>(m_Bitfield));
}

void ADownloadManager::SendPacket(float DeltaTime)
{
	APlayerController* controller = Cast<APlayerController>(GetOwner());
	UNetConnection* NetConnection = controller ? controller->GetNetConnection() : nullptr;

	if (NetConnection)
	{
		bool bLastPacket = false;
		TArray<uint8> sendingData;
		auto nextBit = GetNextPacketData(sendingData);

		// Send the new data to the client (if any exists)
		if (sendingData.Num())
		{
			Client_PostNewPacket(sendingData, BitsetToArray<TRANSFER_BITFIELD_SIZE>(nextBit));
			m_Bitfield |= nextBit;
			UE_LOG(LogNotice, Warning, TEXT("<DownloadManager>: Net connection sent packet"));
		}
	}
}

void ADownloadManager::NotifyDataChanged()
{
	int packetCount = FMath::DivideAndRoundUp(m_Data.Num(), PACKET_SIZE);

	// File is too large to send over
	if (packetCount == 0)
	{
		UE_LOG(LogNotice, Error, TEXT("<DownloadManager>: Notify not sent. Buffer was empty!"));
		return;
	}

	//std::bitset<TRANSFER_BITFIELD_SIZE> ResultantBitField;
	//for (int index = 0; index < packetCount; index++)
	//{
	//	ResultantBitField[index] = true;
	//}

	// Create a struct to hold all the information that will be sent to the clients
	FDownloadInfo newInfo;
	newInfo.PackageSize = m_Data.Num();
	// newInfo.FinalizedBitField = BitsetToArray<TRANSFER_BITFIELD_SIZE>(ResultantBitField);

	// Broadcast the new download information to every client
	m_DownloadInfo = newInfo;

	UE_LOG(LogNotice, Warning, TEXT("<DownloadManager>: New data posted. Size: %i bytes"), m_Data.Num());
	UE_LOG(LogNotice, Warning, TEXT("<DownloadManager>: Total packet count: %i"), packetCount);
}

void ADownloadManager::OnNewDataPosted()
{
	UE_LOG(LogNotice, Warning, TEXT("<DownloadManager>: A new file has been made available on the server"));

	m_Bitfield = 0;
	m_DownloadedSize = 0;
	m_bReadyToDownload = true;

	// Resize the data buffer on the client to be able to hold the incoming data
	m_Data.Empty();
	m_Data.AddUninitialized(m_DownloadInfo.PackageSize);

	// Notify the client that new data is available
	m_NotifyFunc.ExecuteIfBound();
}

std::bitset<TRANSFER_BITFIELD_SIZE> ADownloadManager::GetNextPacketData(TArray<uint8>& Data)
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
		UE_LOG(LogNotice, Error, TEXT("<DownloadManager>:Byte buffer overload! Canceling download..."));
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

void ADownloadManager::Client_PostNewPacket_Implementation(const TArray<uint8>& Data, const TArray<int>& Bitfield)
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
		UE_LOG(LogNotice, Warning, TEXT("<DownloadManager>: Downloading (%i/%i): Bitfield: %s"), m_DownloadedSize, m_Data.Num(), *BitsetStr);

		// Transfer the nessesary data to the correct location in the buffer
		FMemory::Memcpy(m_Data.GetData() + CurrentIndex, Data.GetData(), Data.Num());

		if (m_DownloadedSize == m_Data.Num())
		{
			UE_LOG(LogNotice, Warning, TEXT("<DownloadManager>: Map download complete!"));
			m_bDownloading = false;
			m_bReadyToDownload = false;
		}

	}
	else
	{
		UE_LOG(LogNotice, Warning, TEXT("<DownloadManager>: A packet was ignored, data already sent!"));
	}
}

void ADownloadManager::Client_PostLastNewPacket_Implementation(const TArray<uint8>& Data, const TArray<int>& Bitfield)
{
	Client_PostNewPacket(Data, Bitfield);
}

void ADownloadManager::Server_RequestPacket_Implementation(const TArray<int>& BFRecieved)
{
	UE_LOG(LogNotice, Warning, TEXT("<DownloadManager>: Client request to download acknowldged"));

	// Start the downloading process using the given bitfield
	m_Bitfield = ArrayToBitset<TRANSFER_BITFIELD_SIZE>(BFRecieved);
	m_bDownloading = true;
}

bool ADownloadManager::Server_RequestPacket_Validate(const TArray<int>& BFRecieved)
{
	return true;
}

void ADownloadManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADownloadManager, m_DownloadInfo);
}