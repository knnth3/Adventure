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

	if (Options.Contains("Name="))
	{
		int index = Options.Find("Name=");
		FString remain = Options.RightChop(index);
		int end = remain.Len() - remain.Find(" ");
		remain = remain.LeftChop(end);
		this->MapName = FName(*remain);
		UE_LOG(LogNotice, Warning, TEXT("Map Name: %s"), *remain);
	}

	if (Options.Contains("Size="))
	{
		int index = Options.Find("Size=");
		FString remain = Options.RightChop(index + 5);
		int end = remain.Find(" ");
		remain = remain.LeftChop(end);
		UE_LOG(LogNotice, Warning, TEXT("From: %s"), *remain);

		//First number
		int first = remain.Len() - remain.Find(":");
		FString ColumnString = remain.RightChop(first);
		GridColumns = FCString::Atoi(*ColumnString);
		UE_LOG(LogNotice, Warning, TEXT("Columns: %s"), *ColumnString);

		int last = remain.Len() - first;
		FString RowString = remain.LeftChop(last + 1);
		GridRows = FCString::Atoi(*RowString);
		UE_LOG(LogNotice, Warning, TEXT("Rows: %s"), *RowString);
	}

	FVector Location(0.0f);
	WorldGrid = Cast<AWorldGrid>(GetWorld()->SpawnActor(*GridClass, &Location));

	if (WorldGrid)
	{

		WorldGrid->Initialize(GridRows, GridColumns);

		//Make spawn points
		TArray<FGridCoordinate> SpawnLocations =
		{
		{ 0, 0 },
		{ 0, 1 },
		{ 0, 4 },
		{ 0 ,6 }
		};

		WorldGrid->SetSpawnLocations(SpawnLocations);
	}
	else
	{
		UE_LOG(LogNotice, Error, TEXT("NO WORLD GRID FOUND."));
	}
}
