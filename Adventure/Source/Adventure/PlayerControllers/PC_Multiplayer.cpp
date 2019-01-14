// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "PC_Multiplayer.h"
#include "Grid/PathFinder.h"

APC_Multiplayer::APC_Multiplayer()
{
	UniqueID = -1;
	m_NextPacket = 0;
	m_bMapDownloaded = true;
	m_bNeedsNextPacket = false;
	m_TotalTime = 0;
	m_CurrentDownloadPacketID = 0;
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

	if (m_TotalTime >= 1 && m_bNeedsNextPacket)
	{
		m_TotalTime = 0;
		Client_RecievePacket(GetNextPacketData(), true);
	}
}

int APC_Multiplayer::GetNextPacketData()
{
	return 0;
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

void APC_Multiplayer::Client_RecievePacket_Implementation(int Data, bool LastPacket)
{
	m_CurrentDownloadPacketID++;

	if (LastPacket)
	{
		UE_LOG(LogNotice, Warning, TEXT("<WorldGrid>: Last packet recieved from client."));
		m_bMapDownloaded = true;
	}
	else
	{
		Server_DownloadMap(m_CurrentDownloadPacketID);
	}
}