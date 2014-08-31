#pragma once

// implements logic for the local IDAStealth plugin
// includes client for the remote debugging stealth server
// relies on IDA API being present

#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include "IDAMessageLogger.h"
#include <iostream>
#include <IDAStealthRemote/RemoteStealthClient.h>
#include <IDAStealthRemote/RemoteStealthConnection.h>
#include "RemoteEventListener.h"
#include "ResourceItem.h"
#include <set>
#include "StealthSession.h"

namespace IDAStealth
{
	class LocalStealthSession : public StealthSession
	{
	public:

		LocalStealthSession() :
			resolver_(boost::asio::ip::tcp::resolver(ioService_)),
			remoteSession_(false),
			rtlDispatchExceptionAddr_(0),
			ntContinueCallAddr_(0) {}
		~LocalStealthSession() {}

		void handleDbgAttach(unsigned int processID, const std::string& configFile, const std::string profile);
		void handleProcessStart(unsigned int processID, uintptr_t baseAddress, const std::string& configFile,
			const std::string profile);
		void handleProcessExit();
		void handleBreakPoint(thid_t threadID, ea_t ea);

	private:

		typedef boost::shared_ptr<RemoteStealth::RemoteStealthClient> RemoteStealthClient_Ptr;

		void logString(const std::string& str);
		ResourceItem getRDTSCDriverResource();
		ResourceItem getStealthDriverResource();
		std::string getStealthDllPath();
		void sendRemoteCommand(const RemoteStealth::RSProtocolItem& item);
		void connectToServer();
		std::string readConfigFile(const std::string& fileName);
		bool isWin32RemoteDebugger() const;
		bool isLocalWin32Debugger() const;
		bool isLocalWindbg() const;
		uintptr_t getRtlDispatchExceptionAddr() const;
		uintptr_t getNtContinueCallAddr() const;
		void initSEHMonitoring() const;
		void cleanupSEHMonitoring() const;
		bool setBP(ea_t ea) const;

		IDAMessageLogger logger_;
		RemoteStealthClient_Ptr client_;
		boost::asio::io_service ioService_;
		boost::asio::ip::tcp::resolver resolver_;
		bool remoteSession_;
		RemoteEventListener reListener_;
		
		mutable uintptr_t rtlDispatchExceptionAddr_;
		mutable uintptr_t ntContinueCallAddr_;
		typedef boost::tuple<thid_t, ea_t> BPHit;
		std::set<BPHit> sehHandlerBps_;
		std::set<BPHit> postSEHBps_;
	};
}