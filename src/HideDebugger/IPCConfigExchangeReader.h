#pragma once

#pragma warning(disable : 4189)
#include <boost/interprocess/managed_shared_memory.hpp>
#pragma warning(default : 4189)
#include <iostream>
#include "IDAStealth/IPCConfigExchangeCommon.h"

namespace IPC
{
	class IPCConfigExchangeReader
	{
	public:

		IPCConfigExchangeReader();
		~IPCConfigExchangeReader() {}

		std::string getConfigFile();
		std::string getProfile();
		unsigned int getIDAProcessID();
		IPC::IPCPEHeaderData getIPCPEHeaderData();
		bool isPERestoreRequired();

	private:

		boost::interprocess::managed_shared_memory segment_;
	};
}