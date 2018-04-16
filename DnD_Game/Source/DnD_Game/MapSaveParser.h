#include <typeinfo>
#include <string>
#include <vector>


template<typename T>
class MapSaveParser
{
public:
	MapSaveParser()
	{
		m_typeHash = typeid(T).hash_code();
		m_data.resize(sizeof(size_t));
		memcpy(&m_data[0], (void*)&m_typeHash, sizeof(size_t));
	}

	void push_back(const T& obj)
	{
		int size = m_data.size();
		m_data.resize(m_data.size() + sizeof(T));
		memcpy(&m_data[size], (void*)&obj, sizeof(T));
	}

	std::string to_str()const
	{
		std::string buffer;
		buffer.resize(m_data.size());
		memcpy(&buffer[0], &m_data[0], m_data.size());
		return buffer;
	}

	bool read(const std::string& buffer, std::vector<T>& objects, std::string* error = nullptr, bool bOverwriteSameFileFlag = false)const
	{
		size_t dataHash;
		memcpy(&dataHash, &buffer[0], sizeof(size_t));

		if (dataHash != m_typeHash && !bOverwriteSameFileFlag)
		{
			if (error)
			{
				*error = "Parser type and buffer type mismatch. Enable bOverwriteSameFileFlag to ignore check.";
			}
			return false;
		}

		int dataSize = (buffer.size() - sizeof(size_t));
		int elementCount = dataSize / sizeof(T);

		int bytesRead = 0;
		for (int index = 0; index < elementCount; index++)
		{
			T obj;
			memcpy(&obj, &buffer[bytesRead + sizeof(size_t)], sizeof(T));
			objects.push_back(obj);
			bytesRead += sizeof(T);
		}

		return true;
	}

	bool empty()const
	{
		return (m_data.size() == 0);
	}

private:

	size_t m_typeHash;
	std::vector<unsigned char> m_data;
};