// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "DownloadManager.h"
#include "Adventure.h"

#define PACKET_SIZE (int32)sizeof(FVector)
#define PACKET_TRANSFER_TIME_DELAY 1.0f

TArray<uint8> ADownloadManager::m_Data = TArray<uint8>();

ADownloadManager::ADownloadManager()
{
	m_ElapsedTime = 0;
	bReplicates = true;
	bAlwaysRelevant = true;
	m_DownloadedSize = 0;
	m_bDownloading = false;
	m_bReadyToDownload = false;
	m_bPacketRequested = false;
	PrimaryActorTick.bCanEverTick = true;
}

void ADownloadManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//m_ElapsedTime += DeltaTime;
	//if (m_ElapsedTime >= PACKET_TRANSFER_TIME_DELAY)
	//{
	//	m_ElapsedTime = 0;

	//}

	if (HasAuthority() && m_bPacketRequested)
	{
		SendPacket(DeltaTime);
	}
	else if(!HasAuthority() && m_bDownloading)
	{
		RequestPacket(DeltaTime);
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
}

void ADownloadManager::ServerOnly_NotifyDataChanged()
{
	int packetCount = FMath::DivideAndRoundUp(m_Data.Num(), PACKET_SIZE);

	// File is too large to send over
	if (packetCount == 0)
	{
		UE_LOG(LogNotice, Error, TEXT("<DownloadManager>: Could not set data for download: Buffer is empty"));
		return;
	}

	std::bitset<TRANSFER_BITFIELD_SIZE> ResultantBitField;
	for (int index = 0; index < packetCount; index++)
	{
		ResultantBitField[index] = true;
	}

	// Create a struct to hold all the information that will be sent to the clients
	FDownloadInfo newInfo;
	newInfo.PackageSize = m_Data.Num();
	newInfo.FinalizedBitField = BitsetToArray<TRANSFER_BITFIELD_SIZE>(ResultantBitField);

	// Broadcast the new download information to every client
	m_DownloadInfo = newInfo;

	UE_LOG(LogNotice, Warning, TEXT("<DownloadManager>: New data posted. Size: %i bytes"), m_Data.Num());
	UE_LOG(LogNotice, Warning, TEXT("<DownloadManager>: Total packet count: %i,  Final Bitfield: %s"), packetCount, *FString(ResultantBitField.to_string().c_str()));
}

void ADownloadManager::Subscribe(UNetConnection* connection)
{
	// Set client and server with it's appropriate TCP Roles
	if (HasAuthority())
	{
		m_ConnectionSocket = CreateServerSocket();
	}
	else
	{
		m_ConnectionSocket = CreateClientSocket(connection);
	}
}

void ADownloadManager::BeginDownload()
{
	if (m_bReadyToDownload)
	{
		m_bReadyToDownload = false;
		m_bDownloading = true;
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

FSocket * ADownloadManager::CreateClientSocket(UNetConnection* connection)
{
	if (connection)
	{
		UE_LOG(LogNotice, Warning, TEXT("<DownloadManager>: Estabishing connection"));

		FSocket* listenSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("default"), false);

		if (listenSocket)
		{
			TSharedRef<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
			addr->SetIp(connection->GetAddrAsInt());
			addr->SetPort(connection->GetAddrPort());

			UE_LOG(LogNotice, Warning, TEXT("<DownloadManager>: Requesting connection with %i:%i"), connection->GetAddrAsInt(), connection->GetAddrPort());
			if (listenSocket->Connect(*addr))
			{
				UE_LOG(LogNotice, Warning, TEXT("<DownloadManager>: Connection to TCP server established."));

				return listenSocket;
			}
		}
	}
	return nullptr;
}

FSocket * ADownloadManager::CreateServerSocket()
{
	FString name = "default";
	FString ip = "127.0.0.1";

	uint8 IPv4Nums[4];
	FormatIP4ToNumber(ip, IPv4Nums);

	FIPv4Endpoint Endpoint(FIPv4Address(IPv4Nums[0], IPv4Nums[1], IPv4Nums[2], IPv4Nums[3]), 3478);
	FSocket* listenSocket = FTcpSocketBuilder(*name).AsReusable().BoundToEndpoint(Endpoint).Listening(8);

	if (listenSocket)
	{
		int32 newSize = 0;
		listenSocket->SetReceiveBufferSize(PACKET_SIZE, newSize);

		UE_LOG(LogNotice, Warning, TEXT("<DownloadManager>: Created server TCP"));

		return listenSocket;
	}

	return nullptr;
}

bool ADownloadManager::FormatIP4ToNumber(const FString & TheIP, uint8(&Out)[4])
{
	//IP Formatting
	TheIP.Replace(TEXT(" "), TEXT(""));

	//String Parts
	TArray<FString> Parts;
	TheIP.ParseIntoArray(Parts, TEXT("."), true);
	if (Parts.Num() != 4)
		return false;

	//String to Number Parts
	for (int32 i = 0; i < 4; ++i)
	{
		Out[i] = FCString::Atoi(*Parts[i]);
	}

	return true;
}

void ADownloadManager::RequestPacket(float DeltaTime)
{
	APlayerController* controller = Cast<APlayerController>(GetOwner());
	UNetConnection* NetConnection = controller->GetNetConnection();

	// If the network is ready to send another packet
	if (NetConnection)
	{
		if (NetConnection->IsNetReady(false))
		{
			m_ElapsedTime = 0;

			// Ask for the next packet
			Server_RequestPacket(BitsetToArray<TRANSFER_BITFIELD_SIZE>(m_Bitfield));
			UE_LOG(LogNotice, Warning, TEXT("<DownloadManager>: Net connection sent packet"));
		}
		else
		{
			m_ElapsedTime += DeltaTime;

			if (m_ElapsedTime >= PACKET_TRANSFER_TIME_DELAY)
			{
				m_ElapsedTime = 0;
				UE_LOG(LogNotice, Warning, TEXT("<DownloadManager>: Net connection is not ready to send file."));
			}
		}
	}
}

void ADownloadManager::SendPacket(float DeltaTime)
{
	APlayerController* controller = Cast<APlayerController>(GetOwner());
	auto NetConnection = controller->GetNetConnection();

	TArray<uint8> sendingData;
	auto nextBit = GetNextPacketData(sendingData);

	// Send the new data to the client (if any exists)
	if (sendingData.Num() && NetConnection)
	{
		if (NetConnection->IsNetReady(false))
		{
			m_bPacketRequested = false;
			Client_PostNewPacket(sendingData, BitsetToArray<TRANSFER_BITFIELD_SIZE>(m_Bitfield | nextBit));
			UE_LOG(LogNotice, Warning, TEXT("<DownloadManager>: Net connection sent packet"));
		}
		else
		{
			m_ElapsedTime += DeltaTime;

			if (m_ElapsedTime >= PACKET_TRANSFER_TIME_DELAY)
			{
				m_ElapsedTime = 0;
				UE_LOG(LogNotice, Warning, TEXT("<DownloadManager>: Net connection is not ready to send file."));
			}
		}
	}
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

		// Reserve memory to hold send amount
		Data.AddUninitialized(sendAmnt);

		// Copy memory from save binary to transfer data array
		FMemory::Memcpy(Data.GetData(), m_Data.GetData() + dataIndex, sendAmnt);
	}

	return NextBit;
}

void ADownloadManager::Client_PostNewPacket_Implementation(const TArray<uint8>& Data, const TArray<int>& Bitfield)
{
	// If there are changes to be made
	auto RecievedBitfield = ArrayToBitset<TRANSFER_BITFIELD_SIZE>(Bitfield);

	if (!(m_Bitfield^RecievedBitfield).none())
	{
		auto BFcurrentPacket = (RecievedBitfield | m_Bitfield) & ~m_Bitfield;

		// Get the position in the buffer where the new data should go
		int CurrentIndex = 0;
		for (int x = 0; x < TRANSFER_BITFIELD_SIZE; x++)
		{
			if (BFcurrentPacket == 1)
			{
				CurrentIndex = x * PACKET_SIZE;
				break;
			}
			else
			{
				BFcurrentPacket = BFcurrentPacket >> 1;
			}
		}

		m_DownloadedSize += Data.Num();
		FString BitsetStr(RecievedBitfield.to_string().c_str());
		UE_LOG(LogNotice, Warning, TEXT("<DownloadManager>: Downloading (%i/%i): Bitfield: %s"), m_DownloadedSize, m_Data.Num(), *BitsetStr);

		// Transfer the nessesary data to the correct location in the buffer
		FMemory::Memcpy(m_Data.GetData() + CurrentIndex, Data.GetData(), Data.Num());

		m_Bitfield |= RecievedBitfield;

		if (m_DownloadedSize == m_Data.Num())
		{
			// Build the map
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

void ADownloadManager::Server_RequestPacket_Implementation(const TArray<int>& BFRecieved)
{
	// Set the bitfield to the one recieved from the client
	m_Bitfield = ArrayToBitset<TRANSFER_BITFIELD_SIZE>(BFRecieved);
	m_bPacketRequested = true;
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