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

void ADownloadManager::Subscribe(const FString addr, int port)
{
	UE_LOG(LogNotice, Warning, TEXT("<DownloadManager>: Download starting. Opening connection with %s:%i"), *addr, port);
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

