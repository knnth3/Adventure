// Fill out your copyright notice in the Description page of Project Settings.
#include "GS_Multiplayer.h"

#include "Grid/WorldGrid.h"
#include "Character/MapPawn.h"
#include "UObject/ConstructorHelpers.h"
#include "GameModes/GM_Multiplayer.h"
#include "Adventure.h"


void AGS_Multiplayer::Initialize(FString MapName, int Rows, int Columns)
{
	this->MapName = MapName;
	this->Rows = Rows;
	this->Columns = Columns;
}

void AGS_Multiplayer::HandleBeginPlay()
{
	Super::HandleBeginPlay();

	FVector Location(0.0f);
	WorldGrid = Cast<AWorldGrid>(GetWorld()->SpawnActor(*GridClass, &Location));

	UE_LOG(LogNotice, Warning, TEXT("GameState has begun play!"));

	for (const auto& player : PlayerArray)
	{
		bool Auth = player->HasAuthority();
		FString Name = player->GetPlayerName();
		int ID = player->PlayerId;

		UE_LOG(LogNotice, Warning, TEXT("Found Playerstate: %s with id= %i. IsAuth= %i"), *Name, ID, Auth);

		if (Auth && WorldGrid)
		{
			UE_LOG(LogNotice, Warning, TEXT("Begin World Grid Init"));
			WorldGrid->Initialize(Rows, Columns, player->PlayerId);
			return;
		}
	}
}

void AGS_Multiplayer::BeginPlay()
{
	Super::BeginPlay();
}
