// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "PC_Multiplayer.h"
#include "Grid/PathFinder.h"

#define TRANSFER_DATA_SIZE sizeof(uint32)

APC_Multiplayer::APC_Multiplayer()
{
	UniqueID = -1;
	m_NextPacket = 0;
	m_bMapDownloaded = true;
	m_bNeedsNextPacket = false;
	m_TotalTime = 0;
	m_CurrentDownloadPacketID = 0;
}

void APC_Multiplayer::ServerOnly_SetActiveMapSave(const FString& Path)
{
	if (UBasicFunctions::LoadBinaryFile(Path, m_RawSaveFile))
	{
		UE_LOG(LogNotice, Log, TEXT("<PlayerController>: Current map binary loaded into memory"));
	}
}

void APC_Multiplayer::SetMapName(const FString & Name)
{
	m_MapName = Name;
}

void APC_Multiplayer::SetPlayerID(const int ID)
{
	UniqueID = ID;
}

int APC_Multiplayer::GetPlayerID() const
{
	return UniqueID;
}

void APC_Multiplayer::ShouldDownloadMap(bool bHasMap)
{
	m_bMapDownloaded = bHasMap;
	if (!m_bMapDownloaded)
	{
		Server_DownloadMap(m_CurrentDownloadPacketID);
	}
}

void APC_Multiplayer::ShowPathfindingDebugLines(bool Value)
{
	FPathFinder::ShowDebugPathLines(Value);
}

void APC_Multiplayer::Tick(float DeltaTime)
{
	m_TotalTime += DeltaTime;

	if (!m_bMapDownloaded && m_TotalTime >= 1 && m_bNeedsNextPacket)
	{
		m_TotalTime = 0;

		// Get raw data at m_NextPacket (TRANSFER_DATA_SIZE interval)
		bool lastPacket = false;
		TArray<uint8> Data;
		GetNextPacketData(Data, lastPacket);
		Client_RecievePacket(Data, lastPacket);
	}
}

void APC_Multiplayer::GetNextPacketData(TArray<uint8>& Data, bool& LastPacket)
{
	// How many bytes are left that have not been copied over
	int remain = m_RawSaveFile.Num() - (m_NextPacket * TRANSFER_DATA_SIZE);
	if (remain > 0)
	{
		// Get bytes needed to transfer
		int sendAmnt = (remain > TRANSFER_DATA_SIZE) ? TRANSFER_DATA_SIZE : remain;

		// Should not be less than zero but just in case
		LastPacket = (remain - sendAmnt <= 0);

		// Reserve memory to hold send amount
		Data.AddUninitialized(sendAmnt);

		// Copy memory from save binary to transfer data array
		FMemory::Memcpy(Data.GetData(), Data.GetData() + m_NextPacket * TRANSFER_DATA_SIZE, sendAmnt);
	}
}

void APC_Multiplayer::Server_DownloadMap_Implementation(int packetID)
{
	m_NextPacket = packetID;
	m_bNeedsNextPacket = true;
	m_TotalTime = 0;
}

bool APC_Multiplayer::Server_DownloadMap_Validate(int paketID)
{
	return true;
}

void APC_Multiplayer::Client_RecievePacket_Implementation(const TArray<uint8>& Data, bool LastPacket)
{
	m_CurrentDownloadPacketID++;
	m_RawSaveFile.Append(Data);

	if (LastPacket && !m_bMapDownloaded)
	{
		UE_LOG(LogNotice, Warning, TEXT("<PlayerController>: WorldGrid download complete!"));
		m_bMapDownloaded = true;

		FString path = FString::Printf(TEXT("%sMaps/%s.map"), *FPaths::ProjectUserDir(), *m_MapName);
		UBasicFunctions::SaveBinaryFile(path, m_RawSaveFile);
	}
	else
	{
		Server_DownloadMap(m_CurrentDownloadPacketID);
	}
}