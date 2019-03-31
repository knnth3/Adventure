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

#include "PS_Multiplayer.h"
#include "Grid/WorldGrid.h"
#include <string>

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

void APS_Multiplayer::GenerateEmptyMap(const FString& MapName, const FGridCoordinate & MapSize)
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