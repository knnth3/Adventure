// Fill out your copyright notice in the Description page of Project Settings.

#include "LevelDesignLog.h"
#include "PlatformFilemanager.h"
#include "FileHelper.h"
#include <fstream> 

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
	std::string title(TCHAR_TO_UTF8(*m_levelName));
	//for (const auto& obj : m_placedObjects)
	//{
	//	std::string objName(TCHAR_TO_UTF8(*obj.name));
	//	json temp;
	//	temp[objName]["Position"].push_back(obj.position.X);
	//	temp[objName]["Position"].push_back(obj.position.Y);
	//	temp[objName]["Position"].push_back(obj.position.Z);
	//	m_savedata[title].push_back(temp);
	//}

	//std::stringstream buffer;
	//buffer << std::setw(4) << m_savedata << std::endl;

	FString SaveDirectory = FString(FPlatformProcess::UserDir()) + "/DnD_Game";
	FString FileName = m_levelName + ".json";
	//buffer.str().c_str()
	FString TextToSave = FString();
	bool AllowOverwriting = false;

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	// CreateDirectoryTree returns true if the destination
	// directory existed prior to call or has been created
	// during the call.
	if (PlatformFile.CreateDirectoryTree(*SaveDirectory))
	{
		// Get absolute file path
		FString AbsoluteFilePath = SaveDirectory + "/" + FileName;

		//// Allow overwriting or file doesn't already exist
		//if (AllowOverwriting || !PlatformFile::FileExists(*AbsoluteFilePath))
		//{
		//	FFileHelper::SaveStringToFile(TextToSave, *AbsoluteFilePath);
		//}

		FFileHelper::SaveStringToFile(TextToSave, *AbsoluteFilePath);
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

