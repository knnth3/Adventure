// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "PC_Multiplayer.h"
#include "Grid/PathFinder.h"

APC_Multiplayer::APC_Multiplayer()
{
	UniqueID = -1;
	m_ElapsedTime = 0;
	m_bNewDownloadAvailable = false;
}

void APC_Multiplayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APC_Multiplayer, m_DownloadManager);
}

void APC_Multiplayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (m_bNewDownloadAvailable)
	{
		m_ElapsedTime += DeltaTime;
	}

	// Wait roughly 3 seconds before starting download
	if (m_ElapsedTime >= 3)
	{
		m_ElapsedTime = 0;
		m_bNewDownloadAvailable = false;
		m_DownloadManager->BeginDownload();
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

void APC_Multiplayer::InitNetworkManager()
{
	UWorld* World = GetWorld();
	if (World)
	{
		FActorSpawnParameters params;
		params.Owner = this;

		m_DownloadManager = World->SpawnActor<ADownloadManager>(params);
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

void APC_Multiplayer::OnDownloadManagerCreated()
{
	if (m_DownloadManager)
	{
		FNotifyDelegate del;
		del.BindUObject(this, &APC_Multiplayer::OnNewDataAvailable);
		m_DownloadManager->SetOnDataPostedCallback(del);
	}
}

void APC_Multiplayer::OnNewDataAvailable()
{
	UE_LOG(LogNotice, Warning, TEXT("<PlayerController>: New data is available for download"));
	m_bNewDownloadAvailable = true;
}
