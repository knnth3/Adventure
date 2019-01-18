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
				GenerateGrid(loc);
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

		for (auto& loc : Save->Locations)
		{
			if (loc.Name == CurrentLocation)
			{
				// Serialize location to buffer
				FBufferArchive Buffer(true);
				FMapLocation::StaticStruct()->SerializeBin(Buffer, &loc);
				m_RawSaveFileServer = Buffer;

				FLocationStats stats;
				stats.NameSize = loc.Name.Len();
				stats.HeightMapSize = loc.HeightMap.Num();
				stats.TextureMapSize = loc.TextureMap.Num();
				stats.ObjectsSize = loc.Objects.Num();
				stats.ObjectTransformsSize = loc.ObjectTransforms.Num();

				int packetCount = FMath::DivideAndRoundUp(stats.GetLocationSizeInBytes(), TRANSFER_DATA_SIZE);
				std::bitset<sizeof(int) * 8 * 2> ResultantBitField(pow(2, packetCount) - 1);
				stats.BFFinished = BitsetToArray(ResultantBitField);

				UE_LOG(LogNotice, Warning, TEXT("<PlayerState>: Server has requested Location download: Total packet count: %i,  Final Bitfield: %s"), packetCount, *FString(ResultantBitField.to_string().c_str()));

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

bool APS_Multiplayer::GetLocationFromDownloadBuffer(FMapLocation& Location)
{
	int totalSize = m_LocationStats.NameSize + m_LocationStats.HeightMapSize + m_LocationStats.TextureMapSize + m_LocationStats.ObjectsSize + (sizeof(BasicTransform) * m_LocationStats.ObjectTransformsSize) + sizeof(FGridCoordinate);
	if (totalSize != m_RawSaveFileClient.Num())
	{
		UE_LOG(LogNotice, Warning, TEXT("<PlayerState>: Download does not match data info: Predicted: %i, Actual: %i"), totalSize, m_RawSaveFileClient.Num());
	}

	// Deserialize
	FMemoryReader Reader(m_RawSaveFileClient, true);
	Reader.Seek(0);
	FMapLocation::StaticStruct()->SerializeBin(Reader, &Location);

	return true;
}

void APS_Multiplayer::GenerateGrid(const FMapLocation& Data)
{
	UE_LOG(LogNotice, Warning, TEXT("<PlayerState>: Map loaded: %s <%i, %i>"), *Data.Name, Data.Size.X, Data.Size.Y);
	TActorIterator<AWorldGrid> WorldGrid(GetWorld());
	if (WorldGrid)
	{
		WorldGrid->BuildLocation(Data);
	}
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

			FMapLocation TestResult;
			if (GetLocationFromDownloadBuffer(TestResult))
			{
				GenerateGrid(TestResult);
			}
		}
		else
		{
			// Ask for more data
			UE_LOG(LogNotice, Warning, TEXT("<PlayerState>: Client request sent for packet %i"), (CurrentIndex / TRANSFER_DATA_SIZE) + 1);
			Server_DownloadMap(BitsetToArray(BFIncoming));
		}
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


