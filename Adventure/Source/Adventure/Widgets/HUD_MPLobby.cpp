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

#include "HUD_MPLobby.h"
#include "GameModes/GM_Multiplayer.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"


void AHUD_MPLobby::AddCharacter(FString Username)
{
	OnCharacterConnected(Username);
}

void AHUD_MPLobby::StartSession()
{
	AGM_Multiplayer* Gamemode = Cast<AGM_Multiplayer>(UGameplayStatics::GetGameMode(GetWorld()));
	if (HasAuthority() && Gamemode)
	{
		Gamemode->StartGame();
	}
}

void AHUD_MPLobby::SetMapToLoad(const FString & Name)
{
	AGM_Multiplayer* Gamemode = Cast<AGM_Multiplayer>(UGameplayStatics::GetGameMode(GetWorld()));
	if (HasAuthority() && Gamemode)
	{
		Gamemode->SetMapToLoad(Name);
	}
}

bool AHUD_MPLobby::IsMapSelected()const
{
	AGM_Multiplayer* Gamemode = Cast<AGM_Multiplayer>(UGameplayStatics::GetGameMode(GetWorld()));
	if (HasAuthority() && Gamemode)
	{
		FString MapName;
		Gamemode->GetMapToLoad(MapName);

		return !MapName.IsEmpty();
	}
	return false;
}



