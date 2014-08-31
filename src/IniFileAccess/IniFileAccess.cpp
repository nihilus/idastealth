#include "IniFileAccess.h"
#include <boost/filesystem.hpp>
#include <sstream>

using namespace std;
namespace fs = boost::filesystem;

IniFileAccess::IniFileAccess(const string& iniFile, const string& currentSection) :
	fileName_(iniFile), section_(currentSection)
{
	fs::path path(iniFile);
	path.remove_leaf();
	if (!fs::exists(path)) fs::create_directory(path);
}

IniFileAccess::~IniFileAccess()
{
}

bool IniFileAccess::writeValue(const string& key, int value) const
{
	char tmp[33];
	_itoa_s(value, tmp, 33, 10);
	return writePrivateProfileString(key, string(tmp));
}

bool IniFileAccess::writeValue(const string& key, const string& value) const
{
	return writePrivateProfileString(key, value);
}

bool IniFileAccess::writeValue(const string& key, const BYTE* value, DWORD dataLen) const
{
	char* hexData = new char[2*dataLen+1];
	for (unsigned int i=0; i<dataLen; ++i)
		sprintf_s((char*)&hexData[2*i], 2*dataLen+1, "%02X", value[i]);

	bool retVal = writePrivateProfileString(key, string(hexData));
	delete[] hexData;
	return retVal;
}

bool IniFileAccess::writeValue(const string& key, bool value) const
{
	string tmp = (value == true ? "1" : "0");
	return writePrivateProfileString(key, tmp);
}

bool IniFileAccess::readValue(const string& key, string& value) const
{
	string tmp;
	if (readPrivateProfileString(key, tmp)) 
	{
		value = tmp;
		return true;
	}
	else return false;
}

bool IniFileAccess::readValue(const string& key, int& value) const
{
	string tmp;
	if (readPrivateProfileString(key, tmp))
	{
		value = atoi(tmp.c_str());
		return true;
	}
	else return false;
}

bool IniFileAccess::readValue(const string& key, BYTE* value, DWORD dataLen) const
{
	string tmp;
	if (readPrivateProfileString(key, tmp))
	{		
		stringstream stream;
		int n = 0;
		for (unsigned int i=0; i<dataLen; ++i)
		{
			stream << tmp.substr(2*i, 2);
			stream >> std::hex >> n;
			value[i] = (BYTE)n;
			stream.clear();
		}
		return true;
	}
	else return false;
}

// only modifies value if we could read from inifile
bool IniFileAccess::readValue(const string& key, bool& value) const
{
	string tmp;
	bool retVal = readPrivateProfileString(key, tmp);
	if (retVal) value = (tmp == "1" ? true : false);
	return retVal;
}

// get length of entry
size_t IniFileAccess::getDataLength(const std::string& key)
{
	string tmp;
	readPrivateProfileString(key, tmp);
	return tmp.length();
}

void IniFileAccess::setSection(const string& currentSection)
{
	section_ = currentSection;
}

bool IniFileAccess::writePrivateProfileString(const string& key, const string& value) const
{
	return (WritePrivateProfileString(section_.c_str(), key.c_str(), value.c_str(), fileName_.c_str()) == TRUE ? true : false);
}

bool IniFileAccess::readPrivateProfileString(const string& key, string& value) const
{
	LPTSTR buffer;
	DWORD bufferLen = 255;
	DWORD retLen;
	buffer = (LPTSTR)malloc(bufferLen);
	// determine length of value
	retLen = GetPrivateProfileString(section_.c_str(), key.c_str(), NULL, buffer, bufferLen, fileName_.c_str());
	while(retLen == bufferLen-1)
	{
		bufferLen += bufferLen;
		buffer = (LPTSTR)realloc(buffer, bufferLen);
		retLen = GetPrivateProfileString(section_.c_str(), key.c_str(), NULL, buffer, bufferLen, fileName_.c_str());
	}
	bool retVal = false;
	if (retLen)
	{
		value = string(buffer);
		retVal = true;
	}
	if (buffer) free(buffer);

	return retVal;
}

bool IniFileAccess::wipeSection(const string& section) const
{
	return (WritePrivateProfileString(section.c_str(), NULL, NULL, fileName_.c_str())) ? true : false;
}

vector<string> IniFileAccess::getSections() const
{
	vector<string> sections;
	DWORD bufferSize = 256;
	char* buffer = new char[bufferSize];
	DWORD retVal = GetPrivateProfileSectionNames(buffer, bufferSize, fileName_.c_str());
	while (retVal == bufferSize - 2)
	{
		bufferSize += bufferSize;
		delete[] buffer;
		buffer = new char[bufferSize];
		retVal = GetPrivateProfileSectionNames(buffer, bufferSize, fileName_.c_str());
	}
	
	if (buffer)
	{
		const char* ptr = buffer;
		// keep on walking list event if one of the sections has an empty name
		while(*ptr || (ptr < buffer + retVal))
		{
			size_t len = strlen(ptr);
			if (len) sections.push_back(string(ptr));
			ptr += len + 1;
		}
		delete[] buffer;
	}
	
	return sections;
}