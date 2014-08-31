#pragma once

#include <boost/interprocess/ipc/message_queue.hpp>
#include "RemoteEventCommon.h"
//#include <boost/shared_ptr.hpp>
#include <iostream>

namespace RemoteEvent
{
	class RemoteEventWriter
	{
	public:

		RemoteEventWriter();
		~RemoteEventWriter() {}

		void sendEvent(const RemoteEventData& data) const;

	private:

		boost::interprocess::message_queue mq_;
	};

	//typedef boost::shared_ptr<RemoteEventWriter> RemoteEventWriterPtr; 
}