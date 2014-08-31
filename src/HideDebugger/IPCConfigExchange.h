#pragma once

#pragma warning(disable : 4189)
#include <boost/interprocess/managed_shared_memory.hpp>
#pragma warning(default : 4189)
#include <iostream>
#include <Windows.h>

// TODO: store everything in one struct
struct IPCPEHeaderData 
{
	IPCPEHeaderData() : imageBase(0) {}

	IPCPEHeaderData(uintptr_t base, const IMAGE_NT_HEADERS& headers) :
		imageBase(base),
		ntHeaders(headers) {}

	uintptr_t imageBase;
	IMAGE_NT_HEADERS ntHeaders;
};

class IPCConfigExchangeWriter
{
public:

	IPCConfigExchangeWriter(unsigned int processID);
	~IPCConfigExchangeWriter();
	void setConfigFile(const std::string& configFile);
	void setProfile(const std::string& profile);
	void setIDAProcessID(unsigned int processID);
	void setIPCPEHeaderData(const IPCPEHeaderData& headerData);

private:

	boost::interprocess::managed_shared_memory segment_;
	unsigned int processID_;
};

class IPCConfigExchangeReader
{
public:

	IPCConfigExchangeReader();
	~IPCConfigExchangeReader() {}
	
	std::string getConfigFile();
	std::string getProfile();
	unsigned int getIDAProcessID();
	IPCPEHeaderData getIPCPEHeaderData();

private:

	boost::interprocess::managed_shared_memory segment_;
};