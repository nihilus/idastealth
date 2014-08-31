#include "IPCConfigExchangeReader.h"

using namespace IPC;

IPCConfigExchangeReader::IPCConfigExchangeReader()
{
	using namespace boost::interprocess;
	segment_ = managed_shared_memory(open_read_only, IPC::getSegmentName().c_str());
}

std::string IPCConfigExchangeReader::getConfigFile()
{
	std::pair<char*, size_t> dataPtr = segment_.find<char>(IPC::ConfigFileDataStr);
	return std::string(dataPtr.first);
}

unsigned int IPCConfigExchangeReader::getIDAProcessID()
{
	std::pair<unsigned int*, size_t> dataPtr = segment_.find<unsigned int>(IPC::IDAProcessIDStr);
	return *(dataPtr.first);
}

std::string IPCConfigExchangeReader::getProfile()
{
	std::pair<char*, size_t> dataPtr = segment_.find<char>(IPC::ConfigFileProfileStr);
	return std::string(dataPtr.first);
}

IPC::IPCPEHeaderData IPCConfigExchangeReader::getIPCPEHeaderData()
{
	std::pair<IPC::IPCPEHeaderData*, size_t> dataPtr =
		segment_.find<IPC::IPCPEHeaderData>(IPC::PEHeaderDataStr);
	return *dataPtr.first;
}

bool IPC::IPCConfigExchangeReader::isPERestoreRequired()
{
	std::pair<bool*, size_t> dataPtr = segment_.find<bool>(IPC::PERestoreRequiredStr);
	return *(dataPtr.first);
}