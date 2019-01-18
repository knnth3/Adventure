// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "PS_Multiplayer.h"
#include "Grid/WorldGrid.h"
#include <string>

#define TRANSFER_DATA_SIZE 2048
#define PACKET_TRANSFER_TIME_DELAY 0.02f

APS_Multiplayer::APS_Multiplayer()
{
	SetActorTickEnabled(true);
	m_GameID = -1;
	m_DownloadedSize = 0;
	m_BFSent = 0;
	m_BFRecieved = 0;
	m_CurrentPlayerActive = -1;
	gotAuthority = false;
	m_bMapDownloaded = false;
	m_bNeedsNextPacket = false;
	m_CurrentState = TURN_BASED_STATE::FREE_ROAM;
}

void APS_Multiplayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APS_Multiplayer, m_GameID);
	DOREPLIFETIME(APS_Multiplayer, m_CurrentState);
	DOREPLIFETIME(APS_Multiplayer, m_CurrentPlayerActive);
}

void APS_Multiplayer::ServerOnly_SetGameID(const int ID)
{
	m_GameID = ID;
}

void APS_Multiplayer::ServerOnly_SetPlayerState(const TURN_BASED_STATE state)
{
	m_CurrentState = state;
	OnStateChanged();
}

int APS_Multiplayer::GetGameID() const
{
	return m_GameID;
}

bool APS_Multiplayer::ServerOnly_LoadMap(const FString & MapName)
{
	TActorIterator<AWorldGrid> WorldGrid(GetWorld());
	if (WorldGrid)
	{
		WorldGrid->ServerOnly_SetMapName(MapName);
	}

	FString path = FString::Printf(TEXT("%sMaps/%s.map"), *FPaths::ProjectUserDir(), *MapName);
	UMapSaveFile* Save = Cast<UMapSaveFile>(UBasicFunctions::LoadSaveGameEx(path));
	if (Save)
	{
		FString CurrentLocation;
		for (int x = 0; x < Save->Players.Num(); x++)
		{
			// User found
			if (Save->Players[x] == GetPlayerName())
			{
				CurrentLocation = Save->PlayerLocationNames[x];
				break;
			}
		}

		if (CurrentLocation.IsEmpty())
		{
			CurrentLocation = Save->ActiveLocation;
		}

		for (const auto& loc : Save->Locations)
		{
			if (loc.Name == CurrentLocation)
			{
				UE_LOG(LogNotice, Warning, TEXT("<PlayerState>: Map loaded: %s <%i, %i>"), *loc.Name, loc.Size.X, loc.Size.Y);
				TActorIterator<AWorldGrid> WorldGrid(GetWorld());
				if (WorldGrid)
				{
					WorldGrid->BuildLocation(loc);
				}

				return true;
			}
		}
	}

	return false;
}

bool APS_Multiplayer::LoadMap(const FString& MapName)
{
	TActorIterator<AWorldGrid> WorldGrid(GetWorld());
	if (WorldGrid)
	{
		WorldGrid->ServerOnly_SetMapName(MapName);
	}

	FString path = FString::Printf(TEXT("%sMaps/%s.map"), *FPaths::ProjectUserDir(), *MapName);
	UMapSaveFile* Save = Cast<UMapSaveFile>(UBasicFunctions::LoadSaveGameEx(path));
	if (Save)
	{
		FString CurrentLocation;
		for (int x = 0; x < Save->Players.Num(); x++)
		{
			// User found
			if (Save->Players[x] == GetPlayerName())
			{
				CurrentLocation = Save->PlayerLocationNames[x];
				break;
			}
		}

		if (CurrentLocation.IsEmpty())
		{
			CurrentLocation = Save->ActiveLocation;
		}

		for (const auto& loc : Save->Locations)
		{
			if (loc.Name == CurrentLocation)
			{
				m_CurrentLocation = loc;
				uint32 indexOffset = 0;

				// Write name to buffer
				std::string temp(TCHAR_TO_UTF8(*loc.Name));
				m_RawSaveFileServer.AddUninitialized(temp.size());
				for (const auto& c : temp)
				{
					m_RawSaveFileServer[indexOffset++] = (uint8)c;
				}

				// Write Size
				indexOffset = m_RawSaveFileServer.Num();
				m_RawSaveFileServer.AddUninitialized(sizeof(loc.Size));
				FMemory::Memcpy(m_RawSaveFileServer.GetData() + indexOffset, &loc.Size, sizeof(loc.Size));

				// Write Height map to buffer
				m_RawSaveFileServer.Append(loc.HeightMap);

				// Write Texture map to buffer
				m_RawSaveFileServer.Append(loc.TextureMap);

				// Write Objects to buffer
				m_RawSaveFileServer.Append(loc.Objects);

				// Write Object transforms to buffer
				indexOffset = m_RawSaveFileServer.Num();
				m_RawSaveFileServer.AddUninitialized(loc.ObjectTransforms.Num() * sizeof(BasicTransform));

				for (const auto& transform : loc.ObjectTransforms)
				{
					BasicTransform trans(transform);
					FMemory::Memcpy(m_RawSaveFileServer.GetData() + indexOffset, &trans, sizeof(trans));
					indexOffset += sizeof(trans);
				}

				FLocationStats stats;
				stats.NameSize = loc.Name.Len();
				stats.HeightMapSize = loc.HeightMap.Num();
				stats.TextureMapSize = loc.TextureMap.Num();
				stats.ObjectsSize = loc.Objects.Num();
				stats.ObjectTransformsSize = loc.ObjectTransforms.Num();

				int packetCount = FMath::DivideAndRoundUp(stats.GetLocationSizeInBytes(), TRANSFER_DATA_SIZE);
				std::bitset<sizeof(int) * 8 * 2> tempFinshed(pow(2, packetCount) - 1);
				stats.BFFinished = BitsetToArray(tempFinshed);

				UE_LOG(LogNotice, Warning, TEXT("<PlayerState>: Server has requested Location download: Total packet count: %i,  Final Bitfield: %s"), packetCount, *FString(tempFinshed.to_string().c_str()));

				Client_GetLocationStats(stats);

				// Get raw data at m_NextPacket (TRANSFER_DATA_SIZE interval)
				m_BFSent = 0;
				m_bMapDownloaded = false;
				TArray<uint8> Data;
				GetNextPacketData(Data, m_bMapDownloaded);
				Client_RecievePacket(Data, BitsetToArray(m_BFSent));

				break;
			}
		}

		return true;
	}

	return false;
}

void APS_Multiplayer::GenerateEmptyMap(const FString& MapName, const FGridCoordinate & MapSize)
{
	TActorIterator<AWorldGrid> WorldGrid(GetWorld());
	if (WorldGrid)
	{
		WorldGrid->ServerOnly_SetMapName(MapName);
		Client_GenerateEmptyGrid(MapSize);
	}
}

void APS_Multiplayer::UpdateDataTransfer(float DeltaTime)
{
	if (HasAuthority())
	{
		m_TotalTime += DeltaTime;
		if (!m_bMapDownloaded && m_bNeedsNextPacket && m_TotalTime >= PACKET_TRANSFER_TIME_DELAY)
		{
			m_TotalTime = 0;

			// Get raw data at m_NextPacket (TRANSFER_DATA_SIZE interval)
			TArray<uint8> Data;
			GetNextPacketData(Data, m_bMapDownloaded);
			Client_RecievePacket(Data, BitsetToArray(m_BFSent));
		}
	}
}

void APS_Multiplayer::OverrideCurrentPlayersTurn(const int ID)
{
	m_CurrentPlayerActive = ID;
}

TURN_BASED_STATE APS_Multiplayer::GetCurrentState() const
{
	return m_CurrentState;
}

void APS_Multiplayer::GetNextPacketData(TArray<uint8>& Data, bool & LastPacket)
{
	Data.Empty();

	auto BF = m_BFSent;
	int dataIndex = 0;
	// Get the index of the data to be send
	for (int x = 0; x < m_BFSent.size(); x++)
	{
		// find first open field
		if (BF.to_ullong())
		{
			BF = BF >> 1;
		}
		else
		{
			dataIndex = x * TRANSFER_DATA_SIZE;
			BF = 1;
			BF = BF << x;
			break;
		}
	}

	// How many bytes are left that have not been copied over
	int sendAmnt = 0;
	int remain = m_RawSaveFileServer.Num() - dataIndex;
	if (remain > 0)
	{
		// Get bytes needed to transfer
		sendAmnt = (remain > TRANSFER_DATA_SIZE) ? TRANSFER_DATA_SIZE : remain;

		// Should not be less than zero but just in case
		LastPacket = (remain - sendAmnt <= 0);

		// Reserve memory to hold send amount
		Data.AddUninitialized(sendAmnt);

		// Copy memory from save binary to transfer data array
		FMemory::Memcpy(Data.GetData(), m_RawSaveFileServer.GetData() + dataIndex, sendAmnt);

		// Mark the bit as being sent
		m_BFSent |= BF;
	}
	else
	{
		LastPacket = true;
	}
}

void APS_Multiplayer::LoadLocationDataFromBinary()
{
	int totalSize = m_LocationStats.NameSize + m_LocationStats.HeightMapSize + m_LocationStats.TextureMapSize + m_LocationStats.ObjectsSize + (sizeof(BasicTransform) * m_LocationStats.ObjectTransformsSize) + sizeof(FGridCoordinate);
	if (totalSize != m_RawSaveFileClient.Num())
	{
		UE_LOG(LogNotice, Warning, TEXT("<PlayerState>: Download does not match data info"));
		return;
	}

	FMapLocation Data;
	Data.Name.Reserve(m_LocationStats.NameSize);
	Data.HeightMap.AddUninitialized(m_LocationStats.HeightMapSize);
	Data.TextureMap.AddUninitialized(m_LocationStats.TextureMapSize);
	Data.Objects.AddUninitialized(m_LocationStats.ObjectsSize);
	Data.ObjectTransforms.AddUninitialized(m_LocationStats.ObjectTransformsSize);

	uint32 indexOffset = 0;

	// Read name
	std::string temp;
	for (int x = 0; x < m_LocationStats.NameSize; x++)
	{
		temp.push_back((char)m_RawSaveFileClient[x]);
	}

	Data.Name = FString(UTF8_TO_TCHAR(temp.c_str()));
	indexOffset += m_LocationStats.NameSize;

	// Read Size
	FMemory::Memcpy(&Data.Size, m_RawSaveFileClient.GetData() + indexOffset, sizeof(Data.Size));
	indexOffset += sizeof(Data.Size);

	// Read Height map
	FMemory::Memcpy(Data.HeightMap.GetData(), m_RawSaveFileClient.GetData() + indexOffset, m_LocationStats.HeightMapSize);
	indexOffset += m_LocationStats.HeightMapSize;

	// Read Texture map
	FMemory::Memcpy(Data.TextureMap.GetData(), m_RawSaveFileClient.GetData() + indexOffset, m_LocationStats.TextureMapSize);
	indexOffset += m_LocationStats.TextureMapSize;

	// Read Objects
	FMemory::Memcpy(Data.Objects.GetData(), m_RawSaveFileClient.GetData() + indexOffset, m_LocationStats.ObjectsSize);
	indexOffset += m_LocationStats.ObjectsSize;

	// Read Object transforms
	for (int x = 0; x < m_LocationStats.ObjectTransformsSize; x++)
	{
		BasicTransform trans;
		FMemory::Memcpy(&trans, m_RawSaveFileClient.GetData() + indexOffset, sizeof(BasicTransform));
		indexOffset += sizeof(BasicTransform);

		Data.ObjectTransforms[x] = trans.ToFTransform();
	}

	Client_GenerateGrid(Data);
}

void APS_Multiplayer::Server_DownloadMap_Implementation(const TArray<int>& BFRecieved)
{
	m_bNeedsNextPacket = true;
	m_TotalTime = 0;

}

bool APS_Multiplayer::Server_DownloadMap_Validate(const TArray<int>& BFRecieved)
{
	return true;
}

void APS_Multiplayer::Client_RecievePacket_Implementation(const TArray<uint8>& Data, const TArray<int>& Bitfield)
{

	if (HasAuthority())
	{
		UE_LOG(LogNotice, Warning, TEXT("<PlayerState>: Map download canceled. Running on server"));
		m_bMapDownloaded = true;
		m_bNeedsNextPacket = false;
		Client_GenerateGrid(m_CurrentLocation);
		return;
	}

	// If there are changes to be made
	auto BFIncoming = ArrayToBitset(Bitfield);

	if((m_BFRecieved^BFIncoming).to_ullong())
	{
		auto BFcurrentPacket = (BFIncoming | m_BFRecieved) & ~m_BFRecieved;

		// Get the position in the buffer where the new data should go
		int CurrentIndex = 0;
		for (int x = 0; x < sizeof(int) * 8 * 2; x++)
		{
			if (BFcurrentPacket == 1)
			{
				CurrentIndex = x * TRANSFER_DATA_SIZE;
				break;
			}
			else
			{
				BFcurrentPacket = BFcurrentPacket >> 1;
			}
		}

		m_DownloadedSize += Data.Num();
		FString BitsetStr(BFIncoming.to_string().c_str());
		UE_LOG(LogNotice, Warning, TEXT("<PlayerState>: Downloading (%i/%i): Bitfield: %s"), m_DownloadedSize, m_LocationStats.GetLocationSizeInBytes(), *BitsetStr);

		// Transfer the nessesary data to the correct location in the buffer
		FMemory::Memcpy(m_RawSaveFileClient.GetData() + CurrentIndex, Data.GetData(), Data.Num());

		m_BFRecieved |= BFIncoming;

		auto BFfinished = ArrayToBitset(m_LocationStats.BFFinished);
		if (BFfinished == m_BFRecieved)
		{
			// Build the map
			UE_LOG(LogNotice, Warning, TEXT("<PlayerState>: Map download complete! Packets recieved: %i"), m_RawSaveFileClient.Num());
			LoadLocationDataFromBinary();

		}
		else
		{
			// Ask for more data
			UE_LOG(LogNotice, Warning, TEXT("<PlayerState>: Client request sent for packet %i"), (CurrentIndex / TRANSFER_DATA_SIZE) + 1);
			Server_DownloadMap(BitsetToArray(BFIncoming));
		}
	}
}

void APS_Multiplayer::Client_GenerateGrid(const FMapLocation& Data)
{
	UE_LOG(LogNotice, Warning, TEXT("<PlayerState>: Map loaded: %s <%i, %i>"), *Data.Name, Data.Size.X, Data.Size.Y);
	TActorIterator<AWorldGrid> WorldGrid(GetWorld());
	if (WorldGrid)
	{
		WorldGrid->BuildLocation(Data);
	}
}

void APS_Multiplayer::Client_GenerateEmptyGrid_Implementation(const FGridCoordinate& MapSize)
{
	UE_LOG(LogNotice, Warning, TEXT("<PlayerState>: Generating empty map..."));
	TActorIterator<AWorldGrid> WorldGrid(GetWorld());
	if (WorldGrid)
	{
		WorldGrid->GenerateEmptyLocation(MapSize);
	}
}

void APS_Multiplayer::Client_GetLocationStats_Implementation(const FLocationStats& Stats)
{
	m_DownloadedSize = 0;
	m_LocationStats = Stats;
	m_RawSaveFileClient.AddUninitialized(Stats.GetLocationSizeInBytes());
}


