// Copyright 2019 Eric Marquez
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
// http ://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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

float APC_Multiplayer::GetDownloadPercentage() const
{
	if (m_DownloadManager)
		return m_DownloadManager->GetDataIntegrityPercentage();

	return 0.0f;
}

void APC_Multiplayer::OnPacketReceived()
{
	OnPacketReceivedEvent();
}

void APC_Multiplayer::OnDownloadComplete()
{
	GenerateGrid();
	OnDownloadCompleteEvent();
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
		del.BindUObject(this, &APC_Multiplayer::OnDownloadComplete);
		m_DownloadManager->SetOnDownloadFinishedCallback(del);

		FNotifyDelegate del2;
		del2.BindUObject(this, &APC_Multiplayer::OnPacketReceived);
		m_DownloadManager->SetOnPacketReceivedCallback(del2);

	}
	else
	{
		UE_LOG(LogNotice, Warning, TEXT("<PlayerController>: No download manager existed when trying to bind callback"));
	}
}