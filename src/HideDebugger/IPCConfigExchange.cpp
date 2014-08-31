#include "IPCConfigExchange.h"
#include <sstream>
#include <Windows.h>

namespace ipc = boost::interprocess;
using namespace std;

namespace
{
	const size_t SegmentSize = 2048;
	const char* ConfigFileDataStr = "HideDebuggerPathStrData212";
	const char* ConfigFileProfileStr = "ProfileName";
	const char* IDAProcessIDStr = "IDAProcessID";
	const char* PEHeaderDataStr = "PEHeaderData";
	const size_t ConfigDataSegmentSize = 512;
	const size_t ConfigProfileSegmentSize = 64;

	std::string getSegmentName(unsigned int processID)
	{
		ostringstream oss;
		oss << "HideDebugger" << processID;
		return oss.str();
	}

	std::string getSegmentName()
	{
		return getSegmentName(GetCurrentProcessId());
	}
}

// IPC writer
IPCConfigExchangeWriter::IPCConfigExchangeWriter(unsigned int processID) :
	processID_(processID)
{
	using namespace boost::interprocess;
	string name = getSegmentName(processID);
	try
	{
		ipc::shared_memory_object::remove(name.c_str());
		segment_ = ipc::managed_shared_memory(ipc::create_only, name.c_str(), SegmentSize);
		segment_.construct<char>(ConfigFileDataStr)[ConfigDataSegmentSize](0);
		segment_.construct<char>(ConfigFileProfileStr)[ConfigProfileSegmentSize](0);
		segment_.construct<unsigned int>(IDAProcessIDStr)[1](0);
		segment_.construct<IPCPEHeaderData>(PEHeaderDataStr)[1]();
	}
	catch (...)
	{
		ipc::shared_memory_object::remove(name.c_str());
		throw;
	}
}

IPCConfigExchangeWriter::~IPCConfigExchangeWriter()
{
	ipc::shared_memory_object::remove(getSegmentName(processID_).c_str());
}

void IPCConfigExchangeWriter::setConfigFile(const std::string& configFile)
{
	std::pair<char*, size_t> segmentData = segment_.find<char>(ConfigFileDataStr);
	strcpy_s(segmentData.first, segmentData.second, configFile.c_str());
}

void IPCConfigExchangeWriter::setIDAProcessID(unsigned int processID)
{
	std::pair<unsigned int*, size_t> segmentData = segment_.find<unsigned int>(IDAProcessIDStr);
	*(segmentData.first) = processID;
}

void IPCConfigExchangeWriter::setProfile(const std::string& profile)
{
	std::pair<char*, size_t> segmentData = segment_.find<char>(ConfigFileProfileStr);
	strcpy_s(segmentData.first, segmentData.second, profile.c_str());
}

void IPCConfigExchangeWriter::setIPCPEHeaderData(const IPCPEHeaderData& headerData)
{
	std::pair<IPCPEHeaderData*, size_t> segmentData = segment_.find<IPCPEHeaderData>(PEHeaderDataStr);
	*segmentData.first = headerData;
}

// IPC reader
IPCConfigExchangeReader::IPCConfigExchangeReader()
{
	segment_ = ipc::managed_shared_memory(ipc::open_read_only, getSegmentName().c_str());
}

std::string IPCConfigExchangeReader::getConfigFile()
{
	std::pair<char*, size_t> dataPtr = segment_.find<char>(ConfigFileDataStr);
	return string(dataPtr.first);
}

unsigned int IPCConfigExchangeReader::getIDAProcessID()
{
	std::pair<unsigned int*, size_t> dataPtr = segment_.find<unsigned int>(IDAProcessIDStr);
	return *(dataPtr.first);
}

std::string IPCConfigExchangeReader::getProfile()
{
	std::pair<char*, size_t> dataPtr = segment_.find<char>(ConfigFileProfileStr);
	return string(dataPtr.first);
}

IPCPEHeaderData IPCConfigExchangeReader::getIPCPEHeaderData()
{
	std::pair<IPCPEHeaderData*, size_t> dataPtr = segment_.find<IPCPEHeaderData>(PEHeaderDataStr);
	return *dataPtr.first;
}