#pragma once

#include <iostream>
#include <Windows.h>

class ResourceItem
{
public:
	ResourceItem(HMODULE hModule, int resourceID, const std::string& resourceType);
	~ResourceItem();

	void* getData() const;
	bool saveDataToFile(const std::string& fileName) const;
	size_t getDataSize() const { return size_; };

private:
	HMODULE hModule_;
	int resID_;
	std::string resType_;
	mutable size_t size_;
};