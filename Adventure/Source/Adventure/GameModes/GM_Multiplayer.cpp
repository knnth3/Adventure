// Fill out your copyright notice in the Description page of Project Settings.
#include "GM_Multiplayer.h"

#include "../PlayerControllers/PC_Multiplayer.h"
#include "GI_Adventure.h"
#include "Adventure.h"
#include "Grid/WorldGrid.h"
#include "Character/SpectatorMapPawn.h"
#include "GameStates/GS_Multiplayer.h"
#include "Character/ConnectedPlayer.h"
#include "Saves/MapSaveFile.h"
#include "Widgets/W_GameBuilderUI.h"

FGridCoordinate AGM_Multiplayer::GetMapSize()const
{
	return FGridCoordinate(Rows, Columns);
}

FString AGM_Multiplayer::GetMapName() const
{
	return CurrentMapName;
}

void AGM_Multiplayer::GetMapObjects(TArray<struct FGAMEBUILDER_OBJECT>& Objects) const
{
	Objects = MapDecorations;
}

int AGM_Multiplayer::GetHostID() const
{
	return HostID;
}

void AGM_Multiplayer::InitGame(const FString & MapName, const FString & Options, FString & ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	this->Rows = 10;
	this->Columns = 10;

	FVector Location(0.0f);
	FString MapFileName = UGameplayStatics::ParseOption(Options, "SN");
	UMapSaveFile* MapSaveFile = Cast<UMapSaveFile>(UGameplayStatics::LoadGameFromSlot(MapFileName, 0));
	if (MapSaveFile)
	{
		UE_LOG(LogNotice, Warning, TEXT("Map Loaded!"));
		UE_LOG(LogNotice, Warning, TEXT("Name: %s"), *MapSaveFile->MapName);
		UE_LOG(LogNotice, Warning, TEXT("Size: (%i, %i)"), MapSaveFile->MapSize.X, MapSaveFile->MapSize.Y);
		UE_LOG(LogNotice, Warning, TEXT("Number of Objects: %i"), MapSaveFile->Objects.Num());

		this->Rows = MapSaveFile->MapSize.X;
		this->Columns = MapSaveFile->MapSize.Y;
		MapDecorations = MapSaveFile->Objects;
	}

	APC_Multiplayer* GameHost = Cast<APC_Multiplayer>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (GameHost)
	{
		HostUsername = GameHost->PlayerState->GetPlayerName();
		HostID = GeneratePlayerID();
		GameHost->SetPlayerID(HostID);
		UE_LOG(LogNotice, Warning, TEXT("Found Owner: %s with id= %i."), *HostUsername, HostID);
	}

}

void AGM_Multiplayer::HandleSeamlessTravelPlayer(AController*& NewPlayer)
{
	Super::HandleSeamlessTravelPlayer(NewPlayer);

	APC_Multiplayer* Player = Cast<APC_Multiplayer>(NewPlayer);
	AConnectedPlayer* PlayerActor = Cast<AConnectedPlayer>(NewPlayer->GetPawn());
	if (Player && PlayerActor)
	{
		if (*Player->PlayerState->GetPlayerName() == HostUsername)
		{
			PlayerActor->SetPlayerID(HostID);
			UE_LOG(LogNotice, Warning, TEXT("<HandleSeamlessTravel>: %s: %i"), *Player->PlayerState->GetPlayerName(), HostID);
		}
		else if (PlayerActor->GetPlayerID() == -1)
		{
			int newID = GeneratePlayerID();
			Player->SetPlayerID(newID);
			PlayerActor->SetPlayerID(newID);
			UE_LOG(LogNotice, Warning, TEXT("<HandleSeamlessTravel>: %s: %i"), *Player->PlayerState->GetPlayerName(), newID);
		}

		AConnectedPlayer* playerClass = Cast<AConnectedPlayer>(Player->GetPawn());
		if (playerClass)
		{
			playerClass->AdjustCameraToMap(FGridCoordinate(Rows, Columns));
		}
	}
}

int AGM_Multiplayer::GeneratePlayerID()
{
	return PlayerIndexCount++;
}
