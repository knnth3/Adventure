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
		m_ConnectionSocket = CreateListenSocket(connection);
	}
	else
	{
		m_ConnectionSocket = CreateSendSocket(connection);
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

FSocket * ADownloadManager::CreateListenSocket(UNetConnection* connection)
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

FSocket * ADownloadManager::CreateSendSocket(UNetConnection* connection)
{
	FString name = "default";
	FIPv4Endpoint Endpoint(FIPv4Address(connection->GetAddrAsInt()), connection->GetAddrPort());
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

