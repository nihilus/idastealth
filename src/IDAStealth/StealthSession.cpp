#pragma warning(disable : 4244 4512)
#include <boost/thread.hpp>
#pragma warning(default : 4244 4512)
#include <HideDebugger/HideDebuggerConfig.h>
#include <NInjectLib/IATModifier.h>
#include <NInjectLib/InjectLib.h>
#include <RDTSCEmu/driver/RDTSCEmu.h>
#include "resource.h"
#include <StealthDriver/StealthDriver/StealthDriver.h>
#include "StealthSession.h"
#include <string>
#include <sstream>
#include <Windows.h>

void IDAStealth::StealthSession::handleProcessStart(unsigned int processID, uintptr_t baseAddress,
													const std::string& configFile, const std::string profile)
{
	const HideDebuggerConfig& config = HideDebuggerConfig::getInstance();
	if (config.getEnableDbgStart())
	{
		try
		{
			performCommonInit();
			handleRtlGetNtGlobalFlags(processID);
			
			// now inject stealth dll
			Process process(processID);
			IATModifier iatMod(process);
			iatMod.setImageBase(baseAddress);
			// send config location via IPC
			if (ipc_) ipc_->remove();
			ipc_ = IPCConfigExchangeWriter_Ptr(new IPC::IPCConfigExchangeWriter(processID));
			ipc_->setConfigFile(configFile);
			ipc_->setProfile(profile);
			ipc_->setIPCPEHeaderData(IPC::IPCPEHeaderData(baseAddress, iatMod.readNTHeaders()));
			ipc_->setPERestoreRequired(true);
			iatMod.writeIAT(getStealthDllPath());
		}
		catch (const std::exception& e)
		{
			std::ostringstream oss;
			oss << "Failed to inject stealth dll (" << getStealthDllPath() << ")"
				<< std::endl << "-> " << e.what();
			logString(oss.str());
		}
		catch (...)
		{
			std::ostringstream oss;
			oss << "Unknown error while trying to inject stealth dll (" << getStealthDllPath() << ")";
			logString(oss.str());
		}
	}
}

void IDAStealth::StealthSession::handleRtlGetNtGlobalFlags(unsigned int processID)
{
	HideDebuggerConfig& config = HideDebuggerConfig::getInstance();
	if (!config.getRtlGetNtGlobalFlags()) return;
	
	// assumes that ntdll is loaded to the IBA across processes
	// on ASLR systems, each image is randomized once and is 
	// mapped to the same IBA until reboot
	HMODULE hNtDll = LoadLibrary("ntdll.dll");
	LPVOID address = GetProcAddress(hNtDll, "RtlGetNtGlobalFlags");
	if (address)
	{
		// xor eax, eax; retn
		unsigned char opcodes[] = { 0x31, 0xC0, 0xC3 };
		try
		{
			Process process(processID);
			process.writeMemory(address, opcodes, 3);
		}
		catch (MemoryAccessException& e)
		{
			logString("Error while trying to patch RtlGetNtGlobalFlags: " + std::string(e.what()));
		}
	}
	FreeLibrary(hNtDll);
}

void IDAStealth::StealthSession::performCommonInit()
{
	startDrivers();
}

// possibly start both, the RDTSC and the stealth driver
void IDAStealth::StealthSession::startDrivers()
{
	HideDebuggerConfig& config = HideDebuggerConfig::getInstance();
	
	try
	{		
		// RDTSC driver
		if (config.getUseRDTSC())
		{
			ResourceItem drvResource = getRDTSCDriverResource();
			// empty driver name means: randomize name!
			std::string driverName = config.getRandRDTSCName() ? "" : "RDTSCEMU";
			rdtscDriver_.startDriver(drvResource, driverName);
			RDTSCMode mode = config.getRDTSCMode();
			unsigned int param = mode == constant ? 0 : config.getRDTSCIncrDelta();
			DWORD ioctlCode = mode == constant ? (DWORD)IOCTL_RDTSCEMU_METHOD_ALWAYS_CONST : (DWORD)IOCTL_RDTSCEMU_METHOD_INCREASING;
			rdtscDriver_.setMode(ioctlCode, &param, sizeof(param));
			std::ostringstream oss;
			oss << "Successfully started RDTSC emulation driver from "
				<< rdtscDriver_.getDriverPath().c_str();
			logString(oss.str());
		}
	}
	catch (const std::exception& e)
	{
		logString(e.what());
	}

	try
	{
		// stealth driver
		if (config.getUseStealthDriver())
		{
			ResourceItem stealthDrvResource = getStealthDriverResource();
			stealthDriver_.startDriver(stealthDrvResource, "STEALTHDRIVER");
			std::ostringstream oss;
			oss << "Successfully started stealth driver from "
				<< stealthDriver_.getDriverPath().c_str();
			logString(oss.str());

			StealthHook mode;
			if (config.getStealthNtSetInfoThread())
			{
				mode = SH_NtSetInformationThread;
				stealthDriver_.setMode(IOCTL_STEALTHDRIVER_ENABLE_HOOKS, &mode, sizeof(StealthHook));
			}
			if (config.getStealthNtQueryInfoProcess())
			{
				mode = SH_NtQueryInformationProcess;
				stealthDriver_.setMode(IOCTL_STEALTHDRIVER_ENABLE_HOOKS, &mode, sizeof(StealthHook));
			}
		}
	}
	catch (const std::exception& e)
	{
		logString(e.what());
	}	
}

// possibly stop running driver if allowed by config
void IDAStealth::StealthSession::stopDrivers()
{
	const HideDebuggerConfig& config = HideDebuggerConfig::getInstance();
	if (config.getUnloadRDTSC())
	{
		try
		{
			bool wasRunning = rdtscDriver_.isRunning();
			rdtscDriver_.stopDriver();
			if (wasRunning) logString("Successfully unloaded RDTSC emulation driver");
		}
		catch (const std::exception& e)
		{
			logString("RDTSC driver: " + std::string(e.what()));
		}
	}

	if (config.getUnloadStealth())
	{
		try
		{
			bool wasRunning = stealthDriver_.isRunning();
			stealthDriver_.stopDriver();
			if (wasRunning) logString("Successfully unloaded stealth driver");
		}
		catch (const std::exception& e)
		{
			logString("Stealth driver: " + std::string(e.what()));
		}
	}
}

void IDAStealth::StealthSession::handleDbgAttach(unsigned int processID,
												 const std::string& configFile, const std::string profile)
{
	HideDebuggerConfig& config = HideDebuggerConfig::getInstance();
	if (config.getEnableDbgAttach())
	{
		// we need to start dll injection in background, because the dll injection
		// will block until the process is resumed
		boost::thread(boost::bind(&StealthSession::dbgAttachThread, this, processID, configFile, profile));
	}
}

void IDAStealth::StealthSession::dbgAttachThread(unsigned int processID,
												 const std::string& configFile, const std::string profile)
{
	try
	{
		performCommonInit();
		if (ipc_) ipc_->remove();
		ipc_ = IPCConfigExchangeWriter_Ptr(new IPC::IPCConfigExchangeWriter(processID));
		ipc_->setConfigFile(configFile);
		ipc_->setProfile(profile);
		ipc_->setPERestoreRequired(false);

		Process process(processID);
		InjectLibrary injector(getStealthDllPath(), process);
		bool injected = injector.injectLib();
		if (injected) logString("Successfully injected stealth dll (while attaching to process)");
		else logString("Injection of stealth dll failed (while attaching to process)!");
	}
	catch (const std::exception& e)
	{
		logString(e.what());
	}
}

void IDAStealth::StealthSession::handleProcessExit()
{
	stopDrivers();
}