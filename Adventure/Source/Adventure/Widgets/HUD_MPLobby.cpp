// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "HUD_MPLobby.h"
#include "GameModes/GM_Lobby.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"


void AHUD_MPLobby::AddCharacter(FString Username)
{
	OnCharacterConnected(Username);
}

void AHUD_MPLobby::StartSession()
{
	AGM_Lobby* Gamemode = Cast<AGM_Lobby>(UGameplayStatics::GetGameMode(GetWorld()));
	if (HasAuthority() && Gamemode)
	{
		Gamemode->StartGame();
	}
}

void AHUD_MPLobby::SetMapToLoad(const FString & Name)
{
	AGM_Lobby* Gamemode = Cast<AGM_Lobby>(UGameplayStatics::GetGameMode(GetWorld()));
	if (HasAuthority() && Gamemode)
	{
		Gamemode->SetMapToLoad(Name);
	}
}

bool AHUD_MPLobby::IsMapSelected()const
{
	AGM_Lobby* Gamemode = Cast<AGM_Lobby>(UGameplayStatics::GetGameMode(GetWorld()));
	if (HasAuthority() && Gamemode)
	{
		FString MapName;
		Gamemode->GetMapToLoad(MapName);

		return !MapName.IsEmpty();
	}
	return false;
}



