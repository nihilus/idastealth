#pragma once

// provides serialization of arbitrary objects over boost asio sockets

#pragma warning(disable : 4512)
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#pragma warning(default : 4512)
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <sstream>

namespace RemoteStealth
{
	// provides a mechanism to send/receive our protocol item over a socket
	class RemoteStealthConnection
	{
	public:

		RemoteStealthConnection(boost::asio::io_service& ioService) :
			socket_(ioService) {}
		~RemoteStealthConnection() {}

		boost::asio::ip::tcp::socket& socket() { return socket_; }

		template <typename structT>
		void syncRead(structT& item)
		{
			using namespace std;

			// first read header
			char inboundHeader[HeaderLength];
			boost::asio::read(socket_, boost::asio::buffer(inboundHeader));

			istringstream is(std::string(inboundHeader, HeaderLength));
			size_t inboundDataSize = 0;
			if (!(is >> hex >> inboundDataSize))
				throw runtime_error("Error while processing inbound data: invalid header received");

			// header is correct, so read data
			vector<char> inboundData(inboundDataSize);
			boost::asio::read(socket_, boost::asio::buffer(inboundData));
			try
			{
				string archiveString(&inboundData[0], inboundData.size());
				istringstream archiveStream(archiveString);
				boost::archive::text_iarchive archive(archiveStream);
				archive >> item;
			}
			catch (const exception&)
			{
				throw runtime_error("Error while processing inbound data: unable to deserialize protocol buffer");
			}
		}
		
		template <typename structT>
		void syncWrite(const structT& item)
		{
			using namespace std;

			// serialize data
			ostringstream archiveStream;
			boost::archive::text_oarchive archive(archiveStream);
			archive << item;
			string outboundData = archiveStream.str();

			// create header
			ostringstream headerStream;
			headerStream << setw(HeaderLength) << hex << outboundData.size();
			if (!headerStream || headerStream.str().size() != HeaderLength)
				throw runtime_error("Error while processing outbound data: unable to serialize protocol buffer");

			// gather buffers and write at once
			vector<boost::asio::const_buffer> buffers;
			string headerString = headerStream.str();
			buffers.push_back(boost::asio::buffer(headerString));
			buffers.push_back(boost::asio::buffer(outboundData));
			boost::asio::write(socket_, buffers);
		}

	private:

		boost::asio::ip::tcp::socket socket_;
		static const int HeaderLength = 8;
	};

	typedef boost::shared_ptr<RemoteStealthConnection> RemoteStealthConnectionPtr;
}