#pragma once

// shared functionality between IDAStealth and RemoteStealth to inject the HideDebugger dll
// into a new or already running process
// independent of any IDA API

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include "IPCConfigExchangeWriter.h"
#include <iostream>
#include "ResourceItem.h"
#include "DriverControl.h"

namespace IDAStealth
{
	// encapsulates basic functionality to start IDAStealth as soon as
	// the debuggee starts or when the debugger attaches to the debuggee
	class StealthSession
	{
	public:

		StealthSession() {}
		virtual ~StealthSession() {}

		virtual void handleDbgAttach(unsigned int processID, const std::string& configFile, const std::string profile);
		virtual void handleProcessStart(unsigned int processID, uintptr_t baseAddress,
			const std::string& configFile, const std::string profile);
		virtual void handleProcessExit();

	protected:

		virtual void logString(const std::string& str) =0;
		virtual ResourceItem getRDTSCDriverResource() =0;
		virtual ResourceItem getStealthDriverResource() =0;
		virtual std::string getStealthDllPath() =0;

		void handleRtlGetNtGlobalFlags(unsigned int processID);
		void performCommonInit();
		void startDrivers();
		void stopDrivers();

	private:

		typedef boost::shared_ptr<IPC::IPCConfigExchangeWriter> IPCConfigExchangeWriter_Ptr;

		void dbgAttachThread(unsigned int processID, const std::string& configFile, const std::string profile);

		IPCConfigExchangeWriter_Ptr ipc_;
		DriverControl rdtscDriver_;
		DriverControl stealthDriver_;
	};
}