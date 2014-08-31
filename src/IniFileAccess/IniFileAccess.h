#pragma once

#include <iostream>
#include <vector>
#include <Windows.h>

class IniFileAccess
{
public:
	
	IniFileAccess(const std::string& iniFile, const std::string& currentSection="");
	~IniFileAccess();
	
	void setSection(const std::string& currentSection);
	bool wipeSection(const std::string& section) const;

	bool writeValue(const std::string& key, const std::string& value) const;
	bool writeValue(const std::string& key, int value) const;
	bool writeValue(const std::string& key, const BYTE* value, DWORD dataLen) const;
	bool writeValue(const std::string& key, bool value) const;

	bool readValue(const std::string& key, std::string& value) const;
	bool readValue(const std::string& key, int& vallue) const;
	bool readValue(const std::string& key, BYTE* value, DWORD dataLen) const;
	bool readValue(const std::string& key, bool& value) const;

	size_t getDataLength(const std::string& key);
	std::string getFileName() { return fileName_; }
	std::string getCurrentSection() { return section_; }
	std::vector<std::string> getSections() const;

private:

	bool writePrivateProfileString(const std::string& key, const std::string& value) const;
	bool readPrivateProfileString(const std::string& key, std::string& value) const;
	std::string section_;
	std::string fileName_;
};