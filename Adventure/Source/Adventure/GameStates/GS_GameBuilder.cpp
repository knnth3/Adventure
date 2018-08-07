// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.
#include "GS_GameBuilder.h"

#include "Adventure.h"
#include "Grid/WorldGrid.h"

void AGS_GameBuilder::Initialize(FString MapName, int Rows, int Columns)
{
	this->MapName = MapName;
	this->Rows = Rows;
	this->Columns = Columns;
}

void AGS_GameBuilder::HandleBeginPlay()
{
	Super::HandleBeginPlay();

	FVector Location(0.0f);
	WorldGrid = Cast<AWorldGrid>(GetWorld()->SpawnActor(*GridClass, &Location));

	UE_LOG(LogNotice, Warning, TEXT("GameState has begun play!"));

	for (const auto& player : PlayerArray)
	{
		bool Auth = player->HasAuthority();
		FString Name = player->GetPlayerName();
		int PlayerID = player->PlayerId;

		UE_LOG(LogNotice, Warning, TEXT("Found Playerstate: %s with id= %i. IsAuth= %i"), *Name, PlayerID, Auth);

		if (Auth && WorldGrid)
		{
			FGridCoordinate MapDimensions = FGridCoordinate(Rows, Columns);
			UE_LOG(LogNotice, Warning, TEXT("Begin World Grid Init"));
			WorldGrid->Initialize(PlayerID, MapDimensions);
			return;
		}
	}

}

FGridCoordinate AGS_GameBuilder::GetMapSize() const
{
	return FGridCoordinate(Rows, Columns);
}

FString AGS_GameBuilder::GetMapName() const
{
	return MapName;
}
