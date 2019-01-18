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
	m_ServerSentBitfield = 0;
	m_ClientRecievedBitfield = 0;
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
				// Create a containter to store data that will be sent over
				ULocationSave* Location = Cast<ULocationSave>(UGameplayStatics::CreateSaveGameObject(ULocationSave::StaticClass()));
				Location->LocationData = loc;

				// Pack data into a buffer
				TArray<uint8> Buffer;
				if (UBasicFunctions::ConvertSaveToBinary(Location, Buffer))
				{
					// Store the packed data into the transfer buffer
					m_RawSaveFileServer = Buffer;

					// Give server a heads up of the data it will be recieving
					FLocationStats stats;
					stats.PackageSize = Buffer.Num();

					int packetCount = FMath::DivideAndRoundUp(Buffer.Num(), TRANSFER_DATA_SIZE);
					std::bitset<sizeof(int) * 8 * 2> ResultantBitField(pow(2, packetCount) - 1);
					stats.FinalizedBitField = BitsetToArray(ResultantBitField);

					UE_LOG(LogNotice, Warning, TEXT("<PlayerState>: Server has requested Location download: Total packet count: %i,  Final Bitfield: %s"), packetCount, *FString(ResultantBitField.to_string().c_str()));
					Client_SetLocationStats(stats);

					// Begin packet transfer
					m_ServerSentBitfield = 0;
					m_bMapDownloaded = false;
					TArray<uint8> Data;
					auto NextBit = GetNextPacketData(Data, m_bMapDownloaded);
					Client_RecievePacket(Data, BitsetToArray(m_ServerSentBitfield | NextBit));

					return true;
				}

			}
		}
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

			// Get the next packet that is going to be sent
			TArray<uint8> Data;
			auto NextBit = GetNextPacketData(Data, m_bMapDownloaded);
			Client_RecievePacket(Data, BitsetToArray(m_ServerSentBitfield | NextBit));
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

std::bitset<sizeof(int) * 8 * 2> APS_Multiplayer::GetNextPacketData(TArray<uint8>& Data, bool & LastPacket)
{
	Data.Empty();

	// Get the next bit to represent outgoing package (assumes all bits before it are on)
	std::bitset<sizeof(int) * 8 * 2> NextBit = (m_ServerSentBitfield.to_ullong() + 1);
	int dataIndex = (int)FMath::CeilLogTwo64(NextBit.to_ullong()) * TRANSFER_DATA_SIZE;

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
	}
	else
	{
		LastPacket = true;
	}

	return NextBit;
}

bool APS_Multiplayer::GetLocationFromDownloadBuffer(FMapLocation& Location)
{
	if (m_LocationStats.PackageSize != m_RawSaveFileClient.Num())
	{
		UE_LOG(LogNotice, Warning, TEXT("<PlayerState>: Download does not match data info: Predicted: %i, Actual: %i"), m_LocationStats.PackageSize, m_RawSaveFileClient.Num());
	}

	// Deserialize
	ULocationSave* LocationSave = Cast<ULocationSave>(UBasicFunctions::LoadSaveGameFromBuffer(m_RawSaveFileClient));
	if (LocationSave)
	{
		Location = LocationSave->LocationData;
		return true;
	}


	return false;
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
	m_ServerSentBitfield = ArrayToBitset(BFRecieved);
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
	auto RecievedBitfield = ArrayToBitset(Bitfield);

	if((m_ClientRecievedBitfield^RecievedBitfield).to_ullong())
	{
		auto BFcurrentPacket = (RecievedBitfield | m_ClientRecievedBitfield) & ~m_ClientRecievedBitfield;

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
		FString BitsetStr(RecievedBitfield.to_string().c_str());
		UE_LOG(LogNotice, Warning, TEXT("<PlayerState>: Downloading (%i/%i): Bitfield: %s"), m_DownloadedSize, m_LocationStats.PackageSize, *BitsetStr);

		// Transfer the nessesary data to the correct location in the buffer
		FMemory::Memcpy(m_RawSaveFileClient.GetData() + CurrentIndex, Data.GetData(), Data.Num());

		m_ClientRecievedBitfield |= RecievedBitfield;

		auto BFfinished = ArrayToBitset(m_LocationStats.FinalizedBitField);
		if (BFfinished == m_ClientRecievedBitfield)
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
			Server_DownloadMap(BitsetToArray(RecievedBitfield));
		}
	}
	else
	{
		UE_LOG(LogNotice, Warning, TEXT("<PlayerState>: Server packet ignored, data already sent!"));
		UE_LOG(LogNotice, Warning, TEXT("<PlayerState>: Current Bitfield: %s"), *FString(m_ClientRecievedBitfield.to_string().c_str()));
		UE_LOG(LogNotice, Warning, TEXT("<PlayerState>: Recievd Bitfield: %s"), *FString(RecievedBitfield.to_string().c_str()));
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

void APS_Multiplayer::Client_SetLocationStats_Implementation(const FLocationStats& Stats)
{
	m_ClientRecievedBitfield = 0;
	m_DownloadedSize = 0;
	m_LocationStats = Stats;
	m_RawSaveFileClient.AddUninitialized(Stats.PackageSize);
	UE_LOG(LogNotice, Warning, TEXT("<PlayerState>: Location download has initialized: Download size: %i Bytes"), Stats.PackageSize);
}


