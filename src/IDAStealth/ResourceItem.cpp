#include "ResourceItem.h"

using namespace std;

ResourceItem::ResourceItem(HMODULE hModule, int resourceID, const std::string& resourceType) :
	hModule_(hModule),
	resID_(resourceID),
	resType_(resourceType)
{
}

ResourceItem::~ResourceItem()
{
}

void* ResourceItem::getData() const
{
	size_ = 0;
	HRSRC hResInfo = FindResource(hModule_, (LPCTSTR)resID_, resType_.c_str());
	if (!hResInfo) return NULL;

	HGLOBAL resData = LoadResource(hModule_, hResInfo);
	if (!resData) return NULL;

	void* dataPtr = LockResource(resData);
	if (dataPtr) size_ = SizeofResource(hModule_, hResInfo);
	return dataPtr;
}

bool ResourceItem::saveDataToFile(const string& fileName) const
{
	bool retVal = false;
	void* dataPtr = getData();
	if (!dataPtr) return false;

	FILE* hFile;
	fopen_s(&hFile, fileName.c_str(), "wb");
	if (!hFile) return false;

	if (fwrite(dataPtr, size_, 1, hFile)) retVal = true;
	fclose(hFile);

	return retVal;	
}