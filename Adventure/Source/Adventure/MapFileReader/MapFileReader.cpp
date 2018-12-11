// Fill out your copyright notice in the Description page of Project Settings.
#include "MapFileReader.h"

#include "PlatformFilemanager.h"
#include "FileHelper.h"
#include "FileManager.h"



void FMapSettings::Insert(FName key, FName obj)
{
	auto found = m_names.find(key);
	if (found != m_names.end())
	{
		found->second.push_back(obj);
	}
	else
	{
		m_names[key] = std::vector<FName>();
		m_names[key].push_back(obj);
	}
}

void FMapSettings::Insert(FName key, int obj)
{
	auto found = m_integers.find(key);
	if (found != m_integers.end())
	{
		found->second.push_back(obj);
	}
	else
	{
		m_integers[key] = std::vector<int>();
		m_integers[key].push_back(obj);
	}
}

void FMapSettings::Insert(FName key, float obj)
{
	auto found = m_floats.find(key);
	if (found != m_floats.end())
	{
		found->second.push_back(obj);
	}
	else
	{
		m_floats[key] = std::vector<float>();
		m_floats[key].push_back(obj);
	}
}

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
		for (const auto& sub : names.second)
		{
			FName Tag = names.first;
			FName Value = sub;
			int Type = 1;

			Archive << Tag;
			Archive << Type;
			Archive << Value;
		}
	}

	for (const auto& integers : Settings.GetIntegers())
	{
		for (const auto& sub : integers.second)
		{
			FName Tag = integers.first;
			int Value = sub;
			int Type = 2;

			Archive << Tag;
			Archive << Type;
			Archive << Value;
		}
	}

	for (const auto& floats : Settings.GetFloats())
	{
		for (const auto& sub : floats.second)
		{
			FName Tag = floats.first;
			float Value = sub;
			int Type = 3;

			Archive << Tag;
			Archive << Type;
			Archive << Value;
		}
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