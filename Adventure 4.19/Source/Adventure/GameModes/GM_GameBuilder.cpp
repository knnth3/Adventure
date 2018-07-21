// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.
#include "GM_GameBuilder.h"

#include "GI_Adventure.h"
#include "Grid/WorldGrid.h"

AGM_GameBuilder::AGM_GameBuilder()
{
	static ConstructorHelpers::FClassFinder<AWorldGrid> BP_WorldGrid(TEXT("/Game/Blueprints/Grid/BP_WorldGrid"));
	if (!BP_WorldGrid.Class)
	{
		UE_LOG(LogNotice, Error, TEXT("NO WORLD GRID CLASS FOUND"));
	}

	GridClass = BP_WorldGrid.Class;
}

void AGM_GameBuilder::InitGame(const FString & MapName, const FString & Options, FString & ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	int GridRows = 10;
	int GridColumns = 10;
	m_bNewMap = true;

	//Parse all command options
	FString Name    = ParseStringFor(Options, "Name=",    " ");
	FString Rows    = ParseStringFor(Options, "Rows=",    " ");
	FString Columns = ParseStringFor(Options, "Columns=", " ");
	FString NewMap  = ParseStringFor(Options, "NewMap=",  " ");

	m_MapName = FName(*Name);
	UE_LOG(LogNotice, Warning, TEXT("Map Name: %s"), *Name);

	if (!Columns.IsEmpty() && !Rows.IsEmpty() && Columns.IsNumeric() && Rows.IsNumeric())
	{
		UE_LOG(LogNotice, Warning, TEXT("Map Size: %s x %s"), *Rows, *Columns);

		GridRows = FCString::Atoi(*Rows);
		GridColumns = FCString::Atoi(*Columns);
	}

	if (!NewMap.IsEmpty())
	{
		UE_LOG(LogNotice, Warning, TEXT("New Map: %s"), *NewMap);

		m_bNewMap = (NewMap == "False") ? false : true;
	}

	//Initialize the World Grid
	FVector Location(0.0f);
	WorldGrid = Cast<AWorldGrid>(GetWorld()->SpawnActor(*GridClass, &Location));

	if (WorldGrid)
	{
		WorldGrid->Initialize(GridRows, GridColumns);
	}
	else
	{
		UE_LOG(LogNotice, Error, TEXT("NO WORLD GRID FOUND."));
	}
}

FName AGM_GameBuilder::GetMapName() const
{
	return m_MapName;
}

FGridCoordinate AGM_GameBuilder::GetMapSize() const
{
	if (WorldGrid)
	{
		return WorldGrid->GetSize();
	}

	return FGridCoordinate(0, 0);
}

bool AGM_GameBuilder::IsNewMap() const
{
	return m_bNewMap;
}
