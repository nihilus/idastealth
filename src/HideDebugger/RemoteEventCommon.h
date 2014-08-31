#pragma once

// common data structures for remote event inter process communication

#include <iostream>

namespace RemoteEvent
{
	struct RemoteEventData 
	{
		std::string functionName;
		unsigned int threadID;
	};

	std::string genRemoteEventName(unsigned int processID);
	std::string genRemoteEventName();
}