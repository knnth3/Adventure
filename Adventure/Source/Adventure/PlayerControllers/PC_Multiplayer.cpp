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

void APC_Multiplayer::ShowPathfindingDebugLines(bool Value)
{
	FPathFinder::ShowDebugPathLines(Value);
}