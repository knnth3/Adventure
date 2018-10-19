// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.
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
	FString Name    = ParseStringFor(Options, "Name=",    " ");
	FString Rows    = ParseStringFor(Options, "Rows=",    " ");
	FString Columns = ParseStringFor(Options, "Columns=", " ");
	FString NewMap  = ParseStringFor(Options, "NewMap=",  " ");

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
