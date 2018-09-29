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
	return ConnnectedPlayers.at(HostUsername);
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
}

void AGM_Multiplayer::HandleSeamlessTravelPlayer(AController*& NewPlayer)
{
	Super::HandleSeamlessTravelPlayer(NewPlayer);

	APC_Multiplayer* Player = Cast<APC_Multiplayer>(NewPlayer);
	AConnectedPlayer* PlayerActor = Cast<AConnectedPlayer>(NewPlayer->GetPawn());
	if (Player && PlayerActor)
	{
		std::string PlayerName = TCHAR_TO_UTF8(*Player->PlayerState->GetPlayerName());

		//Host should always connect first
		if (HostUsername.empty())
		{
			HostUsername = PlayerName;
			UE_LOG(LogNotice, Warning, TEXT("<ServerSetup>: Host registered as %s"), *FString(HostUsername.c_str()));
		}

		// New player has joined
		if (ConnnectedPlayers.find(PlayerName) == ConnnectedPlayers.end())
		{
			ConnnectedPlayers[PlayerName] = GeneratePlayerID();
			Player->SetPlayerID(ConnnectedPlayers[PlayerName]);
			PlayerActor->Server_SetPlayerID(ConnnectedPlayers[PlayerName]);
			UE_LOG(LogNotice, Warning, TEXT("<HandleSeamlessTravel>: %s: %i"), *FString(PlayerName.c_str()), PlayerActor->GetPlayerID());
		}
		else
		{
			UE_LOG(LogNotice, Warning, TEXT("<HandleSeamlessTravel>: %s has reconnected via seamless travel?"), *FString(PlayerName.c_str()));
		}

		PlayerActor->Server_AdjustCameraToMap(FGridCoordinate(Rows, Columns));
	}
}

int AGM_Multiplayer::GeneratePlayerID()
{
	return PlayerIndexCount++;
}
