// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "DownloadManager.h"
#include "Adventure.h"

#define PACKET_SIZE 1024

ADownloadManager::ADownloadManager()
{
	bReplicates = true;
	m_dataSize = 0;
}

void ADownloadManager::ServerOnly_SetData(const TArray<uint8>& data)
{
	UE_LOG(LogNotice, Warning, TEXT("<DownloadManager>: Start download process. Download size: %i bytes"), data.Num());

	// Empty old data
	m_data.Empty();
	m_dataSize = data.Num();

	int unpackedCount = data.Num();
	int packetCount = FMath::DivideAndRoundUp(data.Num(), PACKET_SIZE);
	for (int index = 0; index < packetCount; index++)
	{
		// Calculate chunk data
		int size = (unpackedCount < PACKET_SIZE) ? unpackedCount : PACKET_SIZE;
		unpackedCount -= PACKET_SIZE;

		// Create new chunk
		FDownloadChunk newChunk;
		newChunk.PacketID = index;
		newChunk.Data.Insert(&data[index*PACKET_SIZE], size, 0);

		// Add chunk to download buffer
		m_data.Push(newChunk);
	}

	OnDataReceived();
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

TArray<uint8> ADownloadManager::GetUnpackedData() const
{
	TArray<uint8> Unpacked;
	for (const auto& chunk : m_data)
	{
		Unpacked.Insert(chunk.Data, Unpacked.Num());
	}

	return Unpacked;
}

FSocket * ADownloadManager::CreateClientSocket(UNetConnection* connection)
{
	if (connection)
	{
		UE_LOG(LogNotice, Warning, TEXT("<DownloadManager>: Estabishing connection"));

		FSocket* listenSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("default"), false);

		if (listenSocket)
		{
			if (listenSocket->Connect(*connection->GetInternetAddr()))
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

void ADownloadManager::OnDataReceived()
{
	UE_LOG(LogNotice, Warning, TEXT("<DownloadManager>: New Data received. Data size: %i"), m_data.Num());

	if (m_dataSize == m_data.Num())
	{
		UE_LOG(LogNotice, Warning, TEXT("<DownloadManager>: Download completed"));
	}
}

void ADownloadManager::OnDownloadRequested()
{
	UE_LOG(LogNotice, Warning, TEXT("<DownloadManager>: New file have been made available on the server"));
}

void ADownloadManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADownloadManager, m_dataSize);
}