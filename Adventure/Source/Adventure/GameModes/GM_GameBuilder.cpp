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

#include "GM_GameBuilder.h"

#include "GI_Adventure.h"
#include "Grid/WorldGrid.h"
#include "GameStates/GS_GameBuilder.h"
#include "Saves/MapSaveFile.h"
#include "Widgets/W_GameBuilderUI.h"


void AGM_GameBuilder::InitGame(const FString & MapName, const FString & Options, FString & ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	m_GridDimensions = { 10, 10 };
	m_bNewMap = true;

	//Parse all command options
	FString Name    = UGameplayStatics::ParseOption(Options, "MN");
	FString Rows    = UGameplayStatics::ParseOption(Options, "RC");
	FString Columns = UGameplayStatics::ParseOption(Options, "CC");
	FString NewMap  = UGameplayStatics::ParseOption(Options, "NEW");

	UE_LOG(LogNotice, Warning, TEXT("Map Name: %s"), *Name);
	m_MapName = Name;

	if (!NewMap.IsEmpty())
	{
		UE_LOG(LogNotice, Warning, TEXT("New Map: %s"), *NewMap);

		m_bNewMap = (NewMap == "False") ? false : true;
	}

	if (m_bNewMap)
	{
		if (!Columns.IsEmpty() && !Rows.IsEmpty() && Columns.IsNumeric() && Rows.IsNumeric())
		{
			UE_LOG(LogNotice, Warning, TEXT("Map Size: %s x %s"), *Rows, *Columns);

			m_GridDimensions.X = FCString::Atoi(*Rows);
			m_GridDimensions.Y = FCString::Atoi(*Columns);
		}
	}
}

bool AGM_GameBuilder::IsNewMap() const
{
	return m_bNewMap;
}

FGridCoordinate AGM_GameBuilder::GetMapSize() const
{
	return m_GridDimensions;
}

FString AGM_GameBuilder::GetMapName() const
{
	return m_MapName;
}
