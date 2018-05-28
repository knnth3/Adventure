// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

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

	void Insert(FName key, FName obj) { m_names[key] = obj; }
	void Insert(FName key, int obj) { m_integers[key] = obj; }
	void Insert(FName key, float obj) { m_floats[key] = obj; }

	template<typename T>
	T* Get(FName key);

	const std::map<FName, FName>& GetNames()const { return m_names; }
	const std::map<FName, int>& GetIntegers()const { return m_integers; }
	const std::map<FName, float>& GetFloats()const { return m_floats; }

private:
	std::map<FName, FName> m_names;
	std::map<FName, int> m_integers;
	std::map<FName, float> m_floats;
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
