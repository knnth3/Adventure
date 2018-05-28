// Fill out your copyright notice in the Description page of Project Settings.
#include "MapFileReader.h"

#include "PlatformFilemanager.h"
#include "FileHelper.h"
#include "FileManager.h"

MapFileReader::MapFileReader()
{
}

MapFileReader::~MapFileReader()
{
}

bool MapFileReader::SaveMapFile(FString filename, FMapSettings settings)
{
	//Write data to archive. bIsPersistent asks if the file will be written to disk.
	FBufferArchive archive(true);
	SerializeObjects(archive, settings);

	//Grabs the file manager relevant to the current OS
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	//Gets path to (depends on OS) .../Documents/Adventure/
	FString SaveDirectory = FString(FPlatformProcess::UserDir()) + "/Adventure/";
	FString FileName = filename + FILE_EXT;
	FString AbsoluteFilePath = SaveDirectory + FileName;

	//True if found or created
	if (PlatformFile.CreateDirectoryTree(*SaveDirectory))
	{
		//True if success
		if (FFileHelper::SaveArrayToFile(archive, *AbsoluteFilePath))
		{
			// Free Binary Array 	
			archive.FlushCache();
			archive.Empty();

			return true;
		}
	}

	return false;
}

bool MapFileReader::LoadMapFile(FString filename, FMapSettings & Settings)
{
	TArray<uint8> BinaryArray;

	//Grabs the file manager relevant to the current OS
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	//Gets path to (depends on OS) .../Documents/Adventure/
	FString SaveDirectory = FString(FPlatformProcess::UserDir()) + "/Adventure/";
	FString FileName = filename + FILE_EXT;
	FString AbsoluteFilePath = SaveDirectory + FileName;

	//Access denied or not found
	if (!FFileHelper::LoadFileToArray(BinaryArray, *AbsoluteFilePath))
		return false;

	//bIsPersistent asks if the file will be written to disk.
	FMemoryReader Memory(BinaryArray, true);
	Memory.Seek(0);

	// Header is 13 bytes
	DeSerializeObjects(Memory, BinaryArray.Num() - 13, Settings);

	return true;
}

void MapFileReader::DeSerializeObjects(FArchive & Archive, int Size, FMapSettings & Settings)
{
	int FileSize = Size;

	while (FileSize > 0)
	{
		FName Tag;
		Archive << Tag;

		int Type = 0;
		Archive << Type;

		bool inserted = true;
		switch (Type)
		{
		case 1:
		{
			FName variable;
			Archive << variable;
			Settings.Insert(Tag, variable);
		}
			break;

		case 2:
		{
			int variable;
			Archive << variable;
			Settings.Insert(Tag, variable);
		}
			break;

		case 3:
		{
			float variable;
			Archive << variable;
			Settings.Insert(Tag, variable);
		}
			break;

		default:
			inserted = false;
			break;
		}

		if (inserted)
		{
			//1 == FName
			//2 == int
			FileSize = FileSize - GetSizeof(1) - GetSizeof(2) - GetSizeof(Type);
		}
	}
}

void MapFileReader::SerializeObjects(FArchive & Archive, FMapSettings& Settings)
{
	for (const auto& names : Settings.GetNames())
	{
		FName Tag = names.first;
		FName Value = names.second;
		int Type = 1;

		Archive << Tag;
		Archive << Type;
		Archive << Value;
	}

	for (const auto& names : Settings.GetIntegers())
	{
		FName Tag = names.first;
		int Value = names.second;
		int Type = 2;

		Archive << Tag;
		Archive << Type;
		Archive << Value;
	}

	for (const auto& names : Settings.GetFloats())
	{
		FName Tag = names.first;
		float Value = names.second;
		int Type = 3;

		Archive << Tag;
		Archive << Type;
		Archive << Value;
	}
}

int MapFileReader::GetSizeof(int Type)
{
	switch (Type)
	{
	case 1:
		return 8;

	case 2:
		return 4;

	case 3:
		return 4;

	default:
		break;
	}
	return 0;
}
