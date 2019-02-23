// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "PS_Multiplayer.h"
#include "Grid/WorldGrid.h"
#include "DownloadManager/DownloadManager.h"
#include <string>

#define TRANSFER_DATA_SIZE 2048
#define PACKET_TRANSFER_TIME_DELAY 0.02f

APS_Multiplayer::APS_Multiplayer()
{
	SetActorTickEnabled(true);
	m_GameID = -1;
	m_CurrentPlayerActive = -1;
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
		WorldGrid->ServerOnly_AddToInventory(Save->Weapons, Save->Consumables);

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
				// Create a containter to store data that will be sent over
				ULocationSave* Location = Cast<ULocationSave>(UGameplayStatics::CreateSaveGameObject(ULocationSave::StaticClass()));
				Location->LocationData = loc;

				// Pack data into a buffer
				TArray<uint8> Buffer;
				if (UBasicFunctions::ConvertSaveToBinary(Location, Buffer))
				{
					// Send the data to the download manager
					TActorIterator<ADownloadManager> DLManager(GetWorld());
					if (DLManager)
					{
						DLManager->ServerOnly_SetData(Buffer);
					}
				}

				// Load the data on the server
				GenerateGrid(loc);
				return true;
			}
		}
	}

	return false;
}

bool APS_Multiplayer::SetupNetworking()
{
	Client_SetupNetworking();
	return true;
}

void APS_Multiplayer::GenerateEmptyMap(const FString& MapName, const FGridCoordinate & MapSize)
{
}

void APS_Multiplayer::UpdateDataTransfer(float DeltaTime)
{
}

void APS_Multiplayer::OverrideCurrentPlayersTurn(const int ID)
{
	m_CurrentPlayerActive = ID;
}

TURN_BASED_STATE APS_Multiplayer::GetCurrentState() const
{
	return m_CurrentState;
}

bool APS_Multiplayer::GetLocationFromDownloadBuffer(FMapLocation& Location)
{
	// Send the data to the download manager
	TActorIterator<ADownloadManager> DLManager(GetWorld());
	if (DLManager)
	{
		TArray<uint8> LocationData;
		DLManager->GetDataFromBuffer(LocationData);

		// Deserialize
		ULocationSave* LocationSave = Cast<ULocationSave>(UBasicFunctions::LoadSaveGameFromBuffer(LocationData));
		if (LocationSave)
		{
			Location = LocationSave->LocationData;
			return true;
		}
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

void APS_Multiplayer::OnNewDataAvailable()
{
	UE_LOG(LogNotice, Warning, TEXT("<PlayerState>: Starting content download..."));
	TActorIterator<ADownloadManager> DLManager(GetWorld());
	if (DLManager)
	{
		DLManager->BeginDownload();
	}
}

void APS_Multiplayer::Client_SetupNetworking_Implementation()
{
	// Register callback with the download manager
	TActorIterator<ADownloadManager> DLManager(GetWorld());
	if (DLManager)
	{
		FNotifyDelegate callback;
		callback.BindUObject(this, &APS_Multiplayer::OnNewDataAvailable);
		DLManager->SetOnDataPostedCallback(callback);
	}
}