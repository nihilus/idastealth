#pragma once

// implements the IDAStealth remote server

#include <boost/asio.hpp>
#include <iostream>
#include "RemoteStealthConnection.h"
#include "RemoteStealthProtocoll.h"
#include <vector>

namespace RemoteStealth
{
	class RemoteStealthServer
	{
	public:

		RemoteStealthServer(boost::asio::io_service& ioService, unsigned short port);
		~RemoteStealthServer() {}
		void run();

	private:

		void session(RemoteStealthConnectionPtr connection);

		boost::asio::ip::tcp::acceptor acceptor_;
		boost::asio::io_service& ioService_;
		RemoteStealth::RSProtocolItem protocolItem_;
	};
}