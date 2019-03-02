// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "PC_Multiplayer.h"
#include "Grid/PathFinder.h"

#define PACKET_SIZE 256

APC_Multiplayer::APC_Multiplayer()
{
	UniqueID = -1;
	m_ElapsedTime = 0;
	m_bClientPendingDownloadStart = false;
	m_bServerStartTiming = false;
	m_bServerStartDownload = false;
	m_PacketManager = nullptr;
	m_DLPacketInfo.Size = 0;
}

void APC_Multiplayer::BeginPlay()
{
	Super::BeginPlay();

	CreatePacketManager();
}

void APC_Multiplayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Does not run on client
	if (HasAuthority())
	{
		if (m_PacketManager && m_bServerStartDownload)
		{
			m_bServerStartDownload = false;
			if (m_PacketManager->BeginDownload())
			{
				UE_LOG(LogNotice, Warning, TEXT("<PlayerController>: Client has started download"));
				m_bServerStartTiming = true;
			}
			else
			{
				UE_LOG(LogNotice, Warning, TEXT("<PlayerController>: Could not start download. No new data exists."));
			}
		}

		if (m_bServerStartTiming)
		{
			m_ElapsedTime += DeltaTime;
		}

		// Wait at least 3 seconds before starting download
		if (m_ElapsedTime >= 3)
		{
			static int count = 0;
			m_bServerStartTiming = false;

			int packetNum = 0;
			TArray<uint8> SendData;
			if (m_PacketManager->GetNextPacket(SendData, packetNum))
			{
				UE_LOG(LogNotice, Warning, TEXT("<PlayerController>: Sending packet to client. ID: %i"), packetNum);
				Client_PostPacket(SendData, count++);
			}
			else
			{
				// Downloading has finished
				UE_LOG(LogNotice, Warning, TEXT("<PlayerController>: Download has finished"));
				m_ElapsedTime = 0;
			}
		}
	}
}

void APC_Multiplayer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (m_PacketManager)
	{
		m_PacketManager->CleanUp();
		m_PacketManager->Destroy();
	}
}

void APC_Multiplayer::SetupPacketManager()
{
	if (HasAuthority())
	{
		CreatePacketManager();

		if (m_PacketManager)
		{
			FNotifyDelegate del;
			del.BindUObject(this, &APC_Multiplayer::OnDataSetNotify);
			m_PacketManager->SetOnDataPostedCallback(del);
		}
	}
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

void APC_Multiplayer::GenerateGrid()
{
	TActorIterator<AWorldGrid> WorldGrid(GetWorld());
	if (m_PacketManager && WorldGrid)
	{
		TArray<uint8> LocationData;
		m_PacketManager->GetDataFromBuffer(LocationData);
		ULocationSave* LocationSave = Cast<ULocationSave>(UBasicFunctions::LoadSaveGameFromBuffer(LocationData));
		if (LocationSave)
		{
			WorldGrid->BuildLocation(LocationSave->LocationData);
		}
	}
}

void APC_Multiplayer::SetupForDownloading()
{
	if (m_PacketManager)
	{
		UE_LOG(LogNotice, Warning, TEXT("<PlayerController>: Starting Download. Download Size: %i"), m_DLPacketInfo.Size);
		m_bClientPendingDownloadStart = false;
		m_PacketManager->SetIncomingDataInfo(m_DLPacketInfo);
		Server_StartDownload();
	}
	else
	{
		UE_LOG(LogNotice, Error, TEXT("<PlayerController>: A packet manager was unable to be made"));
	}
}

void APC_Multiplayer::CreatePacketManager()
{
	UWorld* World = GetWorld();
	if (World && !m_PacketManager)
	{
		FActorSpawnParameters params;
		params.Owner = this;

		m_PacketManager = World->SpawnActor<APacketManager>(params);

		if (m_bClientPendingDownloadStart)
		{
			SetupForDownloading();
		}
	}
}

void APC_Multiplayer::OnDataSetNotify()
{
	UE_LOG(LogNotice, Warning, TEXT("<PlayerController>: New data is available for download. Total Packets: %i"), m_PacketManager->GetPacketInfo().Size);
	Client_NotifyNewPacketAvailable(m_PacketManager->GetPacketInfo());
}

void APC_Multiplayer::Client_NotifyNewPacketAvailable_Implementation(const FPacketInfo& info)
{
	// Dont run on host
	if (!GetWorld()->IsServer())
	{
		m_DLPacketInfo = info;
		UE_LOG(LogNotice, Warning, TEXT("<PlayerController>: Starting Download. Download size: %i"), info.Size);
		if (m_PacketManager)
		{
			m_bClientPendingDownloadStart = false;
			m_PacketManager->SetIncomingDataInfo(m_DLPacketInfo);
			Server_StartDownload();
		}
		else
		{
			m_bClientPendingDownloadStart = true;
			UE_LOG(LogNotice, Warning, TEXT("<PlayerController>: A packet manager has not been created yet. Postponing call. Download size: %i"), info.Size);
		}
	}
	else
	{
		UE_LOG(LogNotice, Warning, TEXT("<PlayerController>: Notify on client ignored. The client is the host."));
		GenerateGrid();
	}
}

void APC_Multiplayer::Client_PostPacket_Implementation(const TArray<uint8>& data, int packetNum)
{
	if (m_PacketManager)
	{
		if (m_PacketManager->AddPacket(data, packetNum))
		{
			UE_LOG(LogNotice, Warning, TEXT("<PlayerController>: Recieved packet #%i, Byte count: %i. Downloading %f%%"), packetNum, data.Num(), m_PacketManager->GetDataIntegrityPercentage());

			if (m_PacketManager->FinishedDownloading())
			{
				if (m_PacketManager->FinalizeDownload())
				{
					UE_LOG(LogNotice, Warning, TEXT("<PlayerController>: Download Success!"));
					GenerateGrid();
				}
				else
				{
					UE_LOG(LogNotice, Error, TEXT("<PlayerController>: Download failed to finalize!"));
				}
			}
		}
		else
		{
			UE_LOG(LogNotice, Warning, TEXT("<PlayerController>: Recieved packet but was unable to add it. PacketNum: %i"), packetNum);
		}
	}
	else
	{
		UE_LOG(LogNotice, Warning, TEXT("<PlayerController>: A packet manager was not created"));
	}
}

void APC_Multiplayer::Server_StartDownload_Implementation()
{
	UE_LOG(LogNotice, Warning, TEXT("<PlayerController>: Client wants to start downloading..."));
	m_ElapsedTime = 0;
	m_bServerStartDownload = true;
}

bool APC_Multiplayer::Server_StartDownload_Validate()
{
	return true;
}
