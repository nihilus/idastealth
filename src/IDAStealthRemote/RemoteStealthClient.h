#pragma once

// implements the IDAStealth remote client

#include <boost/asio.hpp>
#include <iostream>
#include "RemoteStealthConnection.h"
#include "RemoteStealthProtocoll.h"

namespace RemoteStealth
{
	class RemoteStealthClient
	{
	public:

		RemoteStealthClient(boost::asio::io_service& ioService,
			boost::asio::ip::tcp::resolver::iterator endPointIterator);
		~RemoteStealthClient() {}

		void sendData(const RSProtocolItem& item);
		void connect();

	private:

		void sendShit(const boost::asio::const_buffer& data);

		RemoteStealthClient& operator=(const RemoteStealthClient &);
		boost::asio::io_service& ioService_;
		boost::asio::ip::tcp::resolver::iterator endPointIterator_;
		RemoteStealthConnectionPtr connection_;
	};
}