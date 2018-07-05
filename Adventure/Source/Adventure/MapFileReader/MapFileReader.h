// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <vector>
#include <string>
#include <map>
#include <typeinfo>
#include "Core.h"

#define FILE_EXT ".map"

/**
 * 
 */

class FMapSettings
{
public:

	void Insert(FName key, FName obj);
	void Insert(FName key, int obj);
	void Insert(FName key, float obj);

	template<typename T>
	T* Get(FName key);

	const std::map<FName, std::vector<FName>>& GetNames()const { return m_names; }
	const std::map<FName, std::vector<int>>& GetIntegers()const { return m_integers; }
	const std::map<FName, std::vector<float>>& GetFloats()const { return m_floats; }

	void Clear() { m_names.clear(); m_integers.clear(); m_floats.clear(); }

private:
	std::map<FName, std::vector<FName>> m_names;
	std::map<FName, std::vector<int>> m_integers;
	std::map<FName, std::vector<float>> m_floats;
	std::map<FName, std::vector<FName>> m_positions;
};

class ADVENTURE_API MapFileReader
{
public:
	MapFileReader();
	~MapFileReader();

	static bool SaveMapFile(FString filename, FMapSettings settings);
	static bool LoadMapFile(FString filename, FMapSettings& settings);

private:
	static void DeSerializeObjects(FArchive& Archive, int Size, FMapSettings& Settings);
	static void SerializeObjects(FArchive& Archive, FMapSettings& Settings);
	static int GetSizeof(int Type);
};

template<typename T>
inline T* FMapSettings::Get(FName key)
{
	if (typeid(T).hash_code() == typeid(FName).hash_code())
	{
		if (m_names.find(key) != m_names.end())
		{
			void* value = &m_names[key];
			return (T*)value;
		}
	}
	else if (typeid(T).hash_code() == typeid(int).hash_code())
	{
		if (m_integers.find(key) != m_integers.end())
		{
			void* value = &m_integers[key];
			return (T*)value;
		}
	}
	else if (typeid(T).hash_code() == typeid(float).hash_code())
	{
		if (m_floats.find(key) != m_floats.end())
		{
			void* value = &m_floats[key];
			return (T*)value;
		}
	}

	return nullptr;
}
