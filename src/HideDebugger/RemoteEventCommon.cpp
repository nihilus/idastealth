#include "RemoteEventCommon.h"
#include <sstream>
#include <Windows.h>

namespace RemoteEvent
{
	// gen unique name for message queue object
	std::string genRemoteEventName(unsigned int processID)
	{
		std::ostringstream oss;
		oss << "IDAStealthRemoteEvent" << processID;
		return oss.str();
	}

	std::string genRemoteEventName()
	{
		return genRemoteEventName(GetCurrentProcessId());
	}
}