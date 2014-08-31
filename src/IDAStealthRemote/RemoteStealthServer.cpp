#include <boost/filesystem.hpp>
#pragma warning(disable : 4244 4512)
#include <boost/thread.hpp>
#pragma warning(default : 4244 4512)
#include <boost/bind.hpp>
#include "RemoteStealthServer.h"
#include "RemoteStealthSession.h"

using namespace RemoteStealth;

RemoteStealthServer::RemoteStealthServer(boost::asio::io_service& ioService, unsigned short port) :
	acceptor_(ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
	ioService_(ioService)
{
}

void RemoteStealth::RemoteStealthServer::run()
{
	for (;;)
	{
		RemoteStealthConnectionPtr connection(new RemoteStealthConnection(ioService_));
		acceptor_.accept(connection->socket());
		std::cout << "Accepted connection from " 
				  << connection->socket().remote_endpoint().address()
				  << std::endl;
		boost::thread t(boost::bind(&RemoteStealthServer::session, this, connection));
	}
}

void RemoteStealth::RemoteStealthServer::session(RemoteStealthConnectionPtr connection)
{
	try
	{
		RemoteStealthSession stealthSession;
		for (;;)
		{
			// read via connection stuff
			RSProtocolItem item;
			connection->syncRead(item);
			
			try
			{				
				switch (item.procEvent)
				{
				case ProcessStart:
					std::cout << "process start: process ID = " << item.processID << ", base addr = 0x" << std::hex << item.baseAddress << std::endl;
					stealthSession.handleProcessStart(item);
					break;

				case ProcessAttach:
					stealthSession.handleDbgAttach(item);
					break;

				case ProcessExit:
					stealthSession.handleProcessExit();
					break;
				}
			}
			catch (std::exception& e)
			{
				connection->syncWrite(RSProtocolResponse(false, e.what()));
			}
			// send response
			connection->syncWrite(RSProtocolResponse(true, ""));
			if (item.procEvent == ProcessExit) return;
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error while handling connection: " << e.what() << std::endl;
	}
}