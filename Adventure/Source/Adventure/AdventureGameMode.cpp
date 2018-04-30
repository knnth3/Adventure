// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "AdventureGameMode.h"
#include "AdventureCharacter.h"
#include "UObject/ConstructorHelpers.h"

AAdventureGameMode::AAdventureGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Characters/BasicCharacter/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
