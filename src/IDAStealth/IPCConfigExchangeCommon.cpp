#include "IPCConfigExchangeCommon.h"
#include <sstream>

namespace IPC
{
	std::string getSegmentName(unsigned int processID)
	{
		std::ostringstream oss;
		oss << "HideDebugger" << processID;
		return oss.str();
	}

	std::string getSegmentName()
	{
		return getSegmentName(GetCurrentProcessId());
	}
}