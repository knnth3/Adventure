// Fill out your copyright notice in the Description page of Project Settings.

#include "LevelDesignLog.h"
#include "PlatformFilemanager.h"
#include "FileHelper.h"
#include "FileManager.h"
#include "Serialization/Archive.h"
#include "Runtime/Core/Public/Serialization/ArchiveSaveCompressedProxy.h"
#include "Runtime/Core/Public/Serialization/ArchiveLoadCompressedProxy.h"
#include "Engine/Engine.h"
#include <fstream> 

#define FILE_EXT ".map"

// Sets default values
ALevelDesignLog::ALevelDesignLog()
{
	m_mapRows = 10;
	m_mapColumns = 10;
	PrimaryActorTick.bCanEverTick = false;

}

void ALevelDesignLog::BeginPlay()
{
	Super::BeginPlay();
	
}

void ALevelDesignLog::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALevelDesignLog::AddNewObject(FDecorative newItem)
{
	m_placedObjects.push_back(newItem);
	std::pair<int, int> newPos((int)newItem.position.X, (int)newItem.position.Y);
	m_occupiedGridSpaces[newPos] = true;
}

void ALevelDesignLog::SetLevelName(FString name)
{
	m_levelName = name;
}

void ALevelDesignLog::SetLevelSize(int rows, int columns)
{
	m_mapRows = rows;
	m_mapColumns = columns;
}

void ALevelDesignLog::SetLevelInfo(FString name, int rows, int columns)
{
	m_levelName = name;
	m_mapRows = rows;
	m_mapColumns = columns;
}

bool ALevelDesignLog::SaveLogTo(FString location)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FString SaveDirectory = FString(FPlatformProcess::UserDir()) + "/DnD_Game";
	FString FileName = m_levelName + FILE_EXT;
	std::string title(TCHAR_TO_UTF8(*m_levelName));

	MapFileHeader header;
	header.Columns = m_mapColumns;
	header.Rows = m_mapRows;
	header.Name = m_levelName;

	MapSaveParser<SaveObject> parser(header);

	//Load in all save files before write
	for (const auto& obj : m_placedObjects)
	{
		parser.Push_back(obj.GetSaveObject());
	}

	auto buffer = parser.Data();

	TArray<uint8> saveData;
	saveData.Append((uint8*)&buffer[0], buffer.size());

	// CreateDirectoryTree returns true if the destination
	// directory existed prior to call or has been created
	// during the call.
	if (PlatformFile.CreateDirectoryTree(*SaveDirectory))
	{
		// Get absolute file path
		FString AbsoluteFilePath = SaveDirectory + "/" + FileName;
		FFileHelper::SaveArrayToFile(saveData, *AbsoluteFilePath);
	}

	return true;
}

bool ALevelDesignLog::LoadLogFrom(FString location)
{
	FString SaveDirectory = FString(FPlatformProcess::UserDir()) + "/DnD_Game";
	FString AbsoluteFilePath = SaveDirectory + "/" + location;

	TArray<uint8> saveData;
	if (!FFileHelper::LoadFileToArray(saveData, *AbsoluteFilePath))
		return false;

	uint8_t* temp = saveData.GetData();
	std::vector<byte> buffer(temp, temp + saveData.Num());

	MapSaveParser<SaveObject> parser;

	if (!parser.Load(buffer))
		return false;

	//Clear previous log (if exists) before loading in new info
	ClearLog();

	MapFileHeader header = parser.GetMapHeaderInfo();
	SetLevelName(header.Name.to_fstr());
	SetLevelSize(header.Rows, header.Columns);

	for (const auto& obj : parser.GetObjects())
	{
		FDecorative decor(obj);
		AddNewObject(decor);
	}

	return true;
}

void ALevelDesignLog::ClearLog()
{
	m_mapRows = 0;
	m_mapColumns = 0;
	m_levelName = "";
	m_placedObjects.clear();
	m_occupiedGridSpaces.clear();
}

bool ALevelDesignLog::IsGridSpaceOccupied(FVector2D position) const
{
	std::pair<int, int> newPos((int)position.X, (int)position.Y);
	auto found = m_occupiedGridSpaces.find(newPos);
	if (found != m_occupiedGridSpaces.end())
	{
		return found->second;
	}

	return false;
}

int ALevelDesignLog::GetActorCount() const
{
	return (int)m_placedObjects.size();
}

UStaticMesh * ALevelDesignLog::GetMeshOf(int index) const
{
	UStaticMesh* foundMesh = nullptr;
	if (index < m_placedObjects.size() && index >= 0)
	{
		FString path = "/Game/Models/Ruins/Nature/" + m_placedObjects[index].name;
		foundMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *path));
	}

	return foundMesh;
}

bool ALevelDesignLog::GetInfoOf(int index, FDecorative& obj) const
{
	if (index > m_placedObjects.size() || index < 0)
		return false;

	obj = m_placedObjects[index];
	return true;
}

void ALevelDesignLog::GetLevelInfo(FString & name, int & rows, int & columns) const
{
	name = m_levelName;
	rows = m_mapRows;
	columns = m_mapColumns;
}

void ALevelDesignLog::GetLevelSize(int & rows, int & columns) const
{
	rows = m_mapRows;
	columns = m_mapColumns;
}

void ALevelDesignLog::GetLevelName(FString & name) const
{
	name = m_levelName;
}


