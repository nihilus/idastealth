#pragma once

// provides text serialization of arbitrary objects to be sent
// across the network, process boundaries, etc.

#pragma warning(disable : 4512)
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#pragma warning(default : 4512)
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <sstream>
#include <vector>

namespace RemoteEvent
{
	const int HeaderLength = 8;

	template <typename structT, typename recvHandlerT>
	void deserialize(structT& item, recvHandlerT recv)
	{
		using namespace std;

		// first read header
		char inboundHeader[HeaderLength];
		// read header length
		//boost::asio::read(socket_, boost::asio::buffer(inboundHeader));
		recv(boost::asio::buffer(inboundHeader));

		istringstream is(std::string(inboundHeader, HeaderLength));
		size_t inboundDataSize = 0;
		if (!(is >> hex >> inboundDataSize))
			throw runtime_error("Error while processing inbound data: invalid header received");

		// header is correct, so read data
		vector<char> inboundData(inboundDataSize);
		// read given length
		//boost::asio::read(socket_, boost::asio::buffer(inboundData));
		recv(boost::asio::buffer(inboundData));
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

	template <typename structT, typename transportT>
	void serialize(const structT& item, typename transportT)
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
		// write data to socket/whatever
		//boost::asio::write(socket_, buffers);
		//send(buffers);
	}
}