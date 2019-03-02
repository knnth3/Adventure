// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "PC_Multiplayer.h"
#include "Grid/PathFinder.h"

#define PACKET_SIZE 256

APC_Multiplayer::APC_Multiplayer()
{
	UniqueID = -1;
	m_DownloadManager = nullptr;
}

//sets variables for replicaton over a network
void APC_Multiplayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APC_Multiplayer, m_DownloadManager);
}

void APC_Multiplayer::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		FActorSpawnParameters params;
		params.Owner = this;
		m_DownloadManager = GetWorld()->SpawnActor<ADownloadManager>(params);
		OnNewDownloadManager();
	}
}

void APC_Multiplayer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority() && m_DownloadManager)
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

void APC_Multiplayer::GenerateGrid()
{
	TActorIterator<AWorldGrid> WorldGrid(GetWorld());
	if (m_DownloadManager && WorldGrid)
	{
		TArray<uint8> LocationData;
		m_DownloadManager->GetDataFromBuffer(LocationData);
		ULocationSave* LocationSave = Cast<ULocationSave>(UBasicFunctions::LoadSaveGameFromBuffer(LocationData));
		if (LocationSave)
		{
			WorldGrid->BuildLocation(LocationSave->LocationData);
		}
	}
}

void APC_Multiplayer::OnNewDownloadManager()
{
	if (m_DownloadManager)
	{
		FNotifyDelegate del;
		del.BindUObject(this, &APC_Multiplayer::GenerateGrid);
		m_DownloadManager->SetOnDownloadFinishedCallback(del);

	}
	else
	{
		UE_LOG(LogNotice, Warning, TEXT("<PlayerController>: No download manager existed when trying to bind callback"));
	}
}