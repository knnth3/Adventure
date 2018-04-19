
#include "Runtime/Core/Public/Containers/UnrealString.h"
#include <typeinfo>
#include <string>
#include <vector>

#define HEADER_SIZE sizeof(MapFileHeader)
#define MAP_VERSION 1

typedef unsigned char byte;

class ShortString
{
public:
	ShortString()
	{
	}

	ShortString(FString string)
	{
		int size = (string.Len() < 10) ? string.Len() : 10;
		for (int index = 0; index < size; index++)
		{
			data[index] = string[index];
		}
		data[9] = 0;
	}

	FString to_fstr()const
	{
		return data;
	}

private:
	char data[10] = {};
};

struct MapFileHeader
{
	MapFileHeader()
	{
		Rows = 10;
		Columns = 10;
		Version = MAP_VERSION;
		Hash = 0;
	}

	int Rows;
	int Columns;
	int Version;
	size_t Hash;
	ShortString Name;
};

template<typename T>
class MapSaveParser
{
public:
	MapSaveParser(MapFileHeader header = MapFileHeader())
	{
		m_header = header;
		m_header.Hash = typeid(T).hash_code();
	}

	void SetMapHeaderInfo(MapFileHeader header)
	{
		size_t temp = m_header.Hash;
		m_header = header;
		m_header.Hash = temp;
	}

	MapFileHeader GetMapHeaderInfo()const
	{
		return m_header;
	}

	std::vector<T> GetObjects()const
	{
		return m_data;
	}

	void Push_back(const T& obj)
	{
		m_data.push_back(obj);
	}

	std::vector<byte> Data()const
	{
		int currentIndex = 0;
		std::vector<byte> buffer;

		//Size buffer to be large enough
		int filesize = sizeof(m_header) + (m_data.size() * sizeof(T));
		buffer.resize(filesize);

		//Copies the header
		memcpy(&buffer[0], &m_header, sizeof(m_header));
		currentIndex += sizeof(m_header);

		//Copies the elements
		memcpy(&buffer[currentIndex], &m_data[0], m_data.size() * sizeof(T));

		return buffer;
	}

	bool Load(const std::vector<byte>& buffer, int currentVersion = MAP_VERSION, std::string* error = nullptr, bool bOverwriteSameFileFlag = false)
	{
		int currentIndex = 0;

		if (buffer.size() <= HEADER_SIZE || ((buffer.size() - HEADER_SIZE) % sizeof(T)) != 0)
			return false;

		//Read the header
		MapFileHeader fileHeader;
		memcpy(&fileHeader, &buffer[currentIndex], sizeof(m_header));
		currentIndex += sizeof(m_header);

		if (fileHeader.Version != currentVersion)
			return false;

		if (fileHeader.Hash != m_header.Hash)
			return false;

		//Read the elements
		int dataSize = (buffer.size() - currentIndex);
		int elementCount = dataSize / sizeof(T);

		for (int index = 0; index < elementCount; index++)
		{
			T obj;
			memcpy(&obj, &buffer[currentIndex], sizeof(T));
			m_data.push_back(obj);
			currentIndex += sizeof(T);
		}

		m_header = fileHeader;
		return true;
	}

	bool Empty()const
	{
		return m_data.empty();
	}

private:

	MapFileHeader m_header;
	std::vector<T> m_data;
};