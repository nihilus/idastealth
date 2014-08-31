#include "IPCConfigExchangeWriter.h"

using namespace IPC;

IPCConfigExchangeWriter::IPCConfigExchangeWriter(unsigned int processID) :
	processID_(processID)
{
	using namespace boost::interprocess;
	std::string name = getSegmentName(processID);
	try
	{
		shared_memory_object::remove(name.c_str());
		segment_ = managed_shared_memory(create_only, name.c_str(), SegmentSize);
		segment_.construct<char>(ConfigFileDataStr)[ConfigDataSegmentSize](0);
		segment_.construct<char>(ConfigFileProfileStr)[ConfigProfileSegmentSize](0);
		segment_.construct<unsigned int>(IDAProcessIDStr)[1](0);
		segment_.construct<IPCPEHeaderData>(PEHeaderDataStr)[1]();
		segment_.construct<bool>(PERestoreRequiredStr)[1](false);
	}
	catch (const std::exception&)
	{
		shared_memory_object::remove(name.c_str());
		throw;
	}
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
	std::pair<IPCPEHeaderData*, size_t> segmentData =
		segment_.find<IPCPEHeaderData>(PEHeaderDataStr);
	*segmentData.first = headerData;
}

void IPC::IPCConfigExchangeWriter::setPERestoreRequired(bool required)
{
	std::pair<bool*, size_t> segmentData = segment_.find<bool>(PERestoreRequiredStr);
	*(segmentData.first) = required;
}

void IPCConfigExchangeWriter::remove()
{
	boost::interprocess::shared_memory_object::remove(getSegmentName(processID_).c_str());
}