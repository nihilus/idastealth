#pragma once

#include <iostream>
#include <Windows.h>

namespace IPC
{
	static const size_t SegmentSize = 4096;
	static const char* ConfigFileDataStr = "HideDebuggerPathStrData212";
	static const char* ConfigFileProfileStr = "ProfileName";
	static const char* IDAProcessIDStr = "IDAProcessID";
	static const char* PEHeaderDataStr = "PEHeaderData";
	static const char* PERestoreRequiredStr = "PERestore";
	static const size_t ConfigDataSegmentSize = 2048;
	static const size_t ConfigProfileSegmentSize = 128;

	std::string getSegmentName(unsigned int processID);
	std::string getSegmentName();
	
	// TODO: store everything in one struct (path, profile, etc)
	struct IPCPEHeaderData 
	{
		IPCPEHeaderData() : imageBase(0) {}

		IPCPEHeaderData(uintptr_t base, const IMAGE_NT_HEADERS& headers) :
			imageBase(base),
			ntHeaders(headers) {}

		uintptr_t imageBase;
		IMAGE_NT_HEADERS ntHeaders;
	};
}