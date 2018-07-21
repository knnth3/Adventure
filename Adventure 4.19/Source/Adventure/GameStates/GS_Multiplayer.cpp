// Fill out your copyright notice in the Description page of Project Settings.
#include "GS_Multiplayer.h"

#include "Grid/WorldGrid.h"
#include "Character/MapPawn.h"
#include "UObject/ConstructorHelpers.h"
#include "GameModes/GM_Multiplayer.h"
#include "Adventure.h"


AGS_Multiplayer::AGS_Multiplayer()
{
	static ConstructorHelpers::FClassFinder<AWorldGrid> WorldGridBPClass(TEXT("/Game/Blueprints/Grid/BP_WorldGrid"));
	GridClass = WorldGridBPClass.Class;
}

void AGS_Multiplayer::BeginPlay()
{
	Super::BeginPlay();

}

void AGS_Multiplayer::OnRep_ReplicatedHasBegunPlay()
{
	Super::OnRep_ReplicatedHasBegunPlay();

}

AWorldGrid * AGS_Multiplayer::GetWorldGrid()
{
	return WorldGrid;
}
