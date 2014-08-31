#pragma once

// creates a TCP server to listen for remote events sent from the injected dll
// automatically spawns a background thread to handle the blocking socket calls

#include <boost/shared_ptr.hpp>
#pragma warning(disable : 4244 4512)
#include <boost/thread.hpp>
#pragma warning(default : 4244 4512)
#include <iostream>

namespace IDAStealth
{
	class RemoteEventListener
	{
	public:

		RemoteEventListener();
		~RemoteEventListener();

		std::string getIP() const { return ip_; }
		int getPort() const { return port_; }

	private:

		void backgroundThread();
		std::string ip_;
		int port_;
	};
}