#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "HideDebugger/ObjectTextSerialization.h"
#include "RemoteStealthClient.h"

RemoteStealth::RemoteStealthClient::RemoteStealthClient(boost::asio::io_service& ioService,
														boost::asio::ip::tcp::resolver::iterator endPointIterator) :
	ioService_(ioService)
{
	endPointIterator_ = endPointIterator;
}

void RemoteStealth::RemoteStealthClient::sendData(const RSProtocolItem& item)
{
	// test
	//using namespace RemoteEvent;
	//RSProtocolItem testItem;
	//deserialize(testItem, boost::bind(&RemoteStealthClient::sendShit, this, _1));
	// end test
	connection_->syncWrite(item);
	
	RSProtocolResponse response;
	connection_->syncRead(response);
	if (!response.success)
		throw std::runtime_error("Error while performing remote command: " + response.error);
}

void RemoteStealth::RemoteStealthClient::sendShit(const boost::asio::const_buffer& /*data*/)
{
	// send over boost::asio
}

void RemoteStealth::RemoteStealthClient::connect()
{
	connection_ = RemoteStealthConnectionPtr(new RemoteStealthConnection(ioService_));	
	connection_->socket().connect(*endPointIterator_);
}