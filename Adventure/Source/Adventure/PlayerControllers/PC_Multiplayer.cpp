// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "PC_Multiplayer.h"
#include "Grid/PathFinder.h"

APC_Multiplayer::APC_Multiplayer()
{
	UniqueID = -1;
	m_ElapsedTime = 0;
	m_bNewDownloadAvailable = false;
	m_DownloadManager = nullptr;
}

//sets variables for replicaton over a network
void APC_Multiplayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APC_Multiplayer, m_DLPacketInfo);
}

void APC_Multiplayer::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (World)
	{
		UE_LOG(LogNotice, Warning, TEXT("<%s>: Creating a packet manager"), *GetName());
		FActorSpawnParameters params;
		params.Owner = this;

		m_DownloadManager = World->SpawnActor<APacketManager>(params);

		FNotifyDelegate del;
		del.BindUObject(this, &APC_Multiplayer::OnNewDataAvailable);
		m_DownloadManager->SetOnDataPostedCallback(del);
	}
}

void APC_Multiplayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	m_ElapsedTime += DeltaTime;

	// Wait roughly 3 seconds before starting download
	if (HasAuthority() && !GetWorld()->IsServer())
	{
		if (m_ElapsedTime >= 3 && m_DownloadManager)
		{
			Client_PostPacket(FVector::ZeroVector, 0);
		}

		//if (m_ElapsedTime >= 3)
		//{
		//	TArray<uint8> Data;
		//	TArray<int32> NextBit;
		//	m_DownloadManager->GetSendPacket(Data, NextBit);

		//	Client_PostPacket(FVector::ZeroVector, 0);
		//}

	}
}

void APC_Multiplayer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (m_DownloadManager)
	{
		m_DownloadManager->CleanUp();
		m_DownloadManager->Destroy();
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

void APC_Multiplayer::OnNewDataAvailable()
{
	// Run on anything other than the host's player controller
	if (GetRemoteRole() != Role)
	{
		UE_LOG(LogNotice, Warning, TEXT("<PlayerController>: New data is available for download"));
		m_bNewDownloadAvailable = true;
		m_DLPacketInfo = m_DownloadManager->GetPacketInfo();
	}
}

void APC_Multiplayer::OnBeginDownload()
{
	if (!HasAuthority())
	{
		UE_LOG(LogNotice, Warning, TEXT("<PlayerController>: Starting Download..."));
		if (m_DownloadManager)
		{
			m_DownloadManager->SetIncomingDataInfo(m_DLPacketInfo);
		}
	}
}

void APC_Multiplayer::Client_PostPacket_Implementation(const FVector & data, int packetNum)
{
	if (m_DownloadManager)
	{
		m_DownloadManager->AddPacket();
		UE_LOG(LogNotice, Warning, TEXT("<PlayerController>: Downloading %f%%"), m_DownloadManager->GetDataIntegrityPercentage());
	}
	else
	{
		UE_LOG(LogNotice, Warning, TEXT("<PlayerController>: No packet manager available"));
	}
}
