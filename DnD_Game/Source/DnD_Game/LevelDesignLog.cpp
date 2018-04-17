// Fill out your copyright notice in the Description page of Project Settings.

#include "LevelDesignLog.h"
#include "PlatformFilemanager.h"
#include "FileHelper.h"
#include "MapSaveParser.h"
#include "FileManager.h"
#include "Serialization/Archive.h"
#include "Runtime/Core/Public/Serialization/ArchiveSaveCompressedProxy.h"
#include "Runtime/Core/Public/Serialization/ArchiveLoadCompressedProxy.h"
#include <fstream> 

#define FILE_EXT ".map"

// Sets default values
ALevelDesignLog::ALevelDesignLog()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ALevelDesignLog::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
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

bool ALevelDesignLog::SaveLogTo(FString location)
{
	bool AllowOverwriting = false;

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FString SaveDirectory = FString(FPlatformProcess::UserDir()) + "/DnD_Game";
	FString FileName = m_levelName + FILE_EXT;
	std::string title(TCHAR_TO_UTF8(*m_levelName));


	MapSaveParser<SaveObject> parser;
	TArray<uint8> saveData;

	//Load in all save files before write
	for (const auto& obj : m_placedObjects)
	{
		parser.push_back(obj.GetSaveObject());
	}

	if (parser.empty())
		return false;

	std::string buffer = parser.to_str();
	saveData.Append((uint8*)&buffer[0], buffer.size());


	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::FromInt((int)buffer.size()));
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::FromInt((int)saveData.Num()));
	}

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

bool ALevelDesignLog::LoadLogFrom(FString location)
{
	FString SaveDirectory = FString(FPlatformProcess::UserDir()) + "/DnD_Game";
	FString AbsoluteFilePath = SaveDirectory + "/" + location;

	TArray<uint8> saveData;
	if (!FFileHelper::LoadFileToArray(saveData, *AbsoluteFilePath))
		return false;

	std::string buffer((char*)saveData.GetData(), saveData.Num());

	MapSaveParser<SaveObject> parser;
	std::vector<SaveObject> loadedObjects;
	if (!parser.read(buffer, loadedObjects))
	{
		//TODO: Handle error
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("parser failed"));
		}
		return false;
	}

	location.RemoveFromEnd(TEXT(FILE_EXT));
	SetLevelName(location);

	for (const auto& obj : loadedObjects)
	{
		FDecorative decor(obj);
		AddNewObject(decor);
	}

	return true;
}

int ALevelDesignLog::GetActorCount() const
{
	return (int)m_placedObjects.size();
}

UStaticMesh * ALevelDesignLog::GetMeshOf(int index) const
{
	if (index > m_placedObjects.size() || index < 0)
		return nullptr;

	FString path = "/Game/Models/Ruins/Nature/" + m_placedObjects[index].name;
	return Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, *path));
}

bool ALevelDesignLog::GetInfoOf(int index, FDecorative& obj) const
{
	if (index > m_placedObjects.size() || index < 0)
		return false;

	obj = m_placedObjects[index];
	return true;
}


