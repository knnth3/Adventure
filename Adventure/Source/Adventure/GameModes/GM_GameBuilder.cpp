// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.
#include "GM_GameBuilder.h"

#include "GI_Adventure.h"
#include "Grid/WorldGrid.h"
#include "GameStates/GS_GameBuilder.h"
#include "Saves/MapSaveFile.h"
#include "Widgets/W_GameBuilderUI.h"


void AGM_GameBuilder::InitGame(const FString & MapName, const FString & Options, FString & ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	this->Rows = 10;
	this->Columns = 10;
	m_bNewMap = true;

	//Parse all command options
	FString Name    = ParseStringFor(Options, "Name=",    " ");
	FString Rows    = ParseStringFor(Options, "Rows=",    " ");
	FString Columns = ParseStringFor(Options, "Columns=", " ");
	FString NewMap  = ParseStringFor(Options, "NewMap=",  " ");

	UE_LOG(LogNotice, Warning, TEXT("Map Name: %s"), *Name);
	this->MapName = Name;

	if (!NewMap.IsEmpty())
	{
		UE_LOG(LogNotice, Warning, TEXT("New Map: %s"), *NewMap);

		m_bNewMap = (NewMap == "False") ? false : true;
	}

	if (m_bNewMap)
	{
		if (!Columns.IsEmpty() && !Rows.IsEmpty() && Columns.IsNumeric() && Rows.IsNumeric())
		{
			UE_LOG(LogNotice, Warning, TEXT("Map Size: %s x %s"), *Rows, *Columns);

			this->Rows = FCString::Atoi(*Rows);
			this->Columns = FCString::Atoi(*Columns);
		}
	}
	else
	{
		UMapSaveFile* MapSaveFile = Cast<UMapSaveFile>(UGameplayStatics::LoadGameFromSlot(Name, 0));
		if (MapSaveFile)
		{
			UE_LOG(LogNotice, Warning, TEXT("Map Loaded!"));
			UE_LOG(LogNotice, Warning, TEXT("Name: %s"), *MapSaveFile->MapName);
			UE_LOG(LogNotice, Warning, TEXT("Size: (%i, %i)"), MapSaveFile->MapSize.X, MapSaveFile->MapSize.Y);
			UE_LOG(LogNotice, Warning, TEXT("Number of Objects: %i"), MapSaveFile->Objects.Num());

			this->Rows = MapSaveFile->MapSize.X;
			this->Columns = MapSaveFile->MapSize.Y;
			PendingObjects = MapSaveFile->Objects;
		}
	}
}

void AGM_GameBuilder::InitGameState()
{
	Super::InitGameState();

	AGS_GameBuilder* GameState = GetGameState<AGS_GameBuilder>();
	if (GameState)
	{
		GameState->Initialize(MapName, Rows, Columns);
	}
}

void AGM_GameBuilder::StartPlay()
{
	Super::StartPlay();

	TActorIterator<AWorldGrid> GridItr(GetWorld());
	if (GridItr)
	{
		for (const auto& object : PendingObjects)
		{
			switch (object.Type)
			{
			case GAMEBUILDER_OBJECT_TYPE::ANY:
				break;
			case GAMEBUILDER_OBJECT_TYPE::INTERACTABLE:
				GridItr->AddVisual(object.ModelIndex, object.Location);
				break;
			case GAMEBUILDER_OBJECT_TYPE::SPAWN:
				GridItr->AddSpawnLocation(object.ModelIndex, object.Location);
				break;
			case GAMEBUILDER_OBJECT_TYPE::NPC:
				break;
			default:
				break;
			}
		}
	}

	PendingObjects.Empty();
}

bool AGM_GameBuilder::IsNewMap() const
{
	return m_bNewMap;
}

FGridCoordinate AGM_GameBuilder::GetMapSize() const
{
	return FGridCoordinate(Rows, Columns);
}

FString AGM_GameBuilder::GetMapName() const
{
	return MapName;
}
