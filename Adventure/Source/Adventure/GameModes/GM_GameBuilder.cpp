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

	this->m_Rows = 10;
	this->m_Columns = 10;
	m_bNewMap = true;

	//Parse all command options
	FString Name    = ParseStringFor(Options, "Name=",    " ");
	FString Rows    = ParseStringFor(Options, "Rows=",    " ");
	FString Columns = ParseStringFor(Options, "Columns=", " ");
	FString NewMap  = ParseStringFor(Options, "NewMap=",  " ");

	UE_LOG(LogNotice, Warning, TEXT("Map Name: %s"), *Name);
	this->m_MapName = Name;

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

			this->m_Rows = FCString::Atoi(*Rows);
			this->m_Columns = FCString::Atoi(*Columns);
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

			this->m_Rows = MapSaveFile->MapSize.X;
			this->m_Columns = MapSaveFile->MapSize.Y;
			m_PendingObjects = MapSaveFile->Objects;
		}
	}
}

void AGM_GameBuilder::InitGameState()
{
	Super::InitGameState();

	AGS_GameBuilder* gameState = GetGameState<AGS_GameBuilder>();
	if (gameState)
	{
		gameState->Initialize(m_MapName, m_Rows, m_Columns);
	}
}

void AGM_GameBuilder::StartPlay()
{
	Super::StartPlay();

	TActorIterator<AWorldGrid> GridItr(GetWorld());
	if (GridItr)
	{
		for (const auto& object : m_PendingObjects)
		{
			switch (object.Type)
			{
			case GAMEBUILDER_OBJECT_TYPE::ANY:
				break;
			case GAMEBUILDER_OBJECT_TYPE::INTERACTABLE:
				GridItr->ServerOnly_AddBlockingObject(object.ModelIndex, object.Location);
				break;
			case GAMEBUILDER_OBJECT_TYPE::SPAWN:
				GridItr->ServerOnly_AddSpawnLocation(object.ModelIndex, object.Location);
				break;
			case GAMEBUILDER_OBJECT_TYPE::NPC:
				break;
			default:
				break;
			}
		}
	}

	m_PendingObjects.Empty();
}

bool AGM_GameBuilder::IsNewMap() const
{
	return m_bNewMap;
}

FGridCoordinate AGM_GameBuilder::GetMapSize() const
{
	return FGridCoordinate(m_Rows, m_Columns);
}

FString AGM_GameBuilder::GetMapName() const
{
	return m_MapName;
}
