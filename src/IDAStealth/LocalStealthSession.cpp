// note: since we indirectly include winsock (through boost) this header needs
// to be included before windows.h
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include "NCodeHook/distorm.h"
#include <fstream>
#include "HideDebugger/HideDebuggerConfig.h"
#include "IDACommon.h"
#include "LocalStealthSession.h"
#include <IDAStealthRemote/RemoteStealthConnection.h>
#include "resource.h"
#include <sstream>

namespace
{
	const std::string InjectDllName = "HideDebugger.dll";
}

void IDAStealth::LocalStealthSession::handleDbgAttach(unsigned int processID,
													  const std::string& configFile,
													  const std::string profile)
{
	using namespace RemoteStealth;
	const HideDebuggerConfig& config = HideDebuggerConfig::getInstance();
	if (config.getEnableDbgAttach())
	{
		initSEHMonitoring();
		if (isWin32RemoteDebugger())
		{
			remoteSession_ = true;
			connectToServer();
			sendRemoteCommand(RSProtocolItem(processID, 0, ProcessAttach, readConfigFile(configFile),
				profile, reListener_.getIP(), reListener_.getPort()));
		}
		else if (isLocalWin32Debugger())
		{
			remoteSession_ = false;
			StealthSession::handleDbgAttach(processID, configFile, profile);
		}
	}
}

void IDAStealth::LocalStealthSession::handleProcessStart(unsigned int processID, uintptr_t baseAddress,
														 const std::string& configFile,
														 const std::string profile)
{
	using namespace RemoteStealth;
	const HideDebuggerConfig& config = HideDebuggerConfig::getInstance();
	if (config.getEnableDbgStart())
	{
		initSEHMonitoring();
		if (isWin32RemoteDebugger())
		{
			remoteSession_ = true;
			connectToServer();
			sendRemoteCommand(RSProtocolItem(processID, baseAddress, ProcessStart, readConfigFile(configFile),
				profile, reListener_.getIP(), reListener_.getPort()));
		}
		else if (isLocalWin32Debugger())
		{
			remoteSession_ = false;
			StealthSession::handleProcessStart(processID, baseAddress, configFile, profile);
		}
	}
}

void IDAStealth::LocalStealthSession::handleProcessExit()
{
	using namespace RemoteStealth;
	const HideDebuggerConfig& config = HideDebuggerConfig::getInstance();
	if (config.getEnableDbgStart() || config.getEnableDbgAttach())
	{
		cleanupSEHMonitoring();
		if (isWin32RemoteDebugger() && remoteSession_)
		{
			sendRemoteCommand(RSProtocolItem(0, 0, ProcessExit, "", "", "", 0));
		}
		else if (isLocalWin32Debugger()) StealthSession::handleProcessExit();
	}
}

void IDAStealth::LocalStealthSession::logString(const std::string& str)
{
	logger_.log("IDAStealth: " + str + "\n");
}

ResourceItem IDAStealth::LocalStealthSession::getRDTSCDriverResource()
{
	return ResourceItem(GetModuleHandle("IDAStealth.plw"), IDR_RDTSC, "DRV");
}

ResourceItem IDAStealth::LocalStealthSession::getStealthDriverResource()
{
	return ResourceItem(GetModuleHandle("IDAStealth.plw"), IDR_STEALTH, "DRV");
}

std::string IDAStealth::LocalStealthSession::getStealthDllPath()
{
	boost::filesystem::path retVal;
	char idaExe[MAX_PATH];
	if (GetModuleFileName(NULL, idaExe, MAX_PATH))
	{
		boost::filesystem::path p(idaExe);
		p.remove_leaf();
		p /= "plugins";
		p /= InjectDllName;
		return p.native_file_string();
	}
	return "";
}

void IDAStealth::LocalStealthSession::sendRemoteCommand(const RemoteStealth::RSProtocolItem& item)
{
	using namespace boost::asio;

	try
	{
		client_->sendData(item);
	}
	catch (const std::exception& e)
	{
		logString("Error while sending remote command: " + std::string(e.what()));
	}
}

void IDAStealth::LocalStealthSession::connectToServer()
{
	using namespace boost::asio;

	try
	{
		// get host for remote debugging
		qstring host;
		get_process_options(NULL, NULL, NULL, &host, NULL, NULL);

		const HideDebuggerConfig& config = HideDebuggerConfig::getInstance();
		std::ostringstream oss;
		oss << config.getRemoteTCPPort();

		ip::tcp::resolver::query query(host.c_str(), oss.str());
		ip::tcp::resolver::iterator iterator = resolver_.resolve(query);

		client_ = RemoteStealthClient_Ptr(new RemoteStealth::RemoteStealthClient(ioService_, iterator));
		client_->connect();
	}
	catch (const std::exception& e)
	{
		logString("Error while connecting: " + std::string(e.what()));
	}
}

std::string IDAStealth::LocalStealthSession::readConfigFile(const std::string& fileName)
{
	std::ifstream ifs(fileName.c_str());
	std::ostringstream oss;
	oss << ifs.rdbuf();
	return oss.str();
}

bool IDAStealth::LocalStealthSession::isWin32RemoteDebugger() const
{
	// we must not return true for remote windbg aka kernel debugger
	return dbg->is_remote() && dbg->id == DEBUGGER_ID_X86_IA32_WIN32_USER;
}

bool IDAStealth::LocalStealthSession::isLocalWin32Debugger() const
{
	return (!dbg->is_remote() && dbg->id == DEBUGGER_ID_X86_IA32_WIN32_USER) ||
		isLocalWindbg();
}

bool IDAStealth::LocalStealthSession::isLocalWindbg() const
{
	netnode nn("$ windbg_params");
	ulong value = nn.altval(2);
	return !value;
}

void IDAStealth::LocalStealthSession::handleBreakPoint(thid_t threadID, ea_t ea)
{
	const HideDebuggerConfig& config = HideDebuggerConfig::getInstance();
	std::set<BPHit>::const_iterator sehCit = sehHandlerBps_.find(BPHit(threadID, ea));
	std::set<BPHit>::const_iterator postSEHCit = postSEHBps_.find(BPHit(threadID, ea));
	if (ea == getRtlDispatchExceptionAddr() && (config.getHaltInSEH() || config.getLogSEH()))
	{
		// read first entry from SEH chain and set BP at handler
		ea_t fsBase = 0;
		regval_t fsRegVal;
		get_reg_val("fs", &fsRegVal);
		if (dbg->thread_get_sreg_base(threadID, (int)fsRegVal.ival, &fsBase) != 1)
			throw std::runtime_error("Failed to determine top-level SEH handler in RtlDispatchException");
		
		// now read pointer to top level SEH record
		uintptr_t sehChain = 0;
		uintptr_t sehHandler = 0;
		if (dbg->read_memory(fsBase, &sehChain, sizeof(sehChain)) == sizeof(sehChain) &&
			dbg->read_memory(sehChain + sizeof(uintptr_t), &sehHandler, sizeof(sehHandler)) == sizeof(sehHandler))
		{
			if (!setBP(sehHandler)) throw std::runtime_error("Error while setting breakpoint at top-level SEH handler in RtlDispatchException");
			sehHandlerBps_.insert(BPHit(threadID, sehHandler));
		}
		else throw std::runtime_error("Error while reading memory to determine top-level SEH handler in RtlDispatchException");
		continue_process();
	}
	else if (ea == getNtContinueCallAddr() && (config.getHaltAfterSEH() || config.getLogSEH()))
	{
		// the first parameter on the stack is a pointer to the CONTEXT structure
		regval_t espVal;
		if (get_reg_val("esp", &espVal))
		{
			uintptr_t contextAddr;
			CONTEXT ctx;
			if (dbg->read_memory((int)espVal.ival, &contextAddr, sizeof(contextAddr)) == sizeof(contextAddr) &&
				dbg->read_memory(contextAddr, &ctx, sizeof(ctx)) == sizeof(ctx))
			{
				if (!setBP(ctx.Eip)) throw std::runtime_error("Error while setting breakpoint at modified instruction pointer after SEH");
				postSEHBps_.insert(BPHit(threadID, ctx.Eip));
			}
		}
		else throw std::runtime_error("Unable to get stack value while trying to determine modified instruction pointer after SEH");
		continue_process();
	}
	else if (sehCit != sehHandlerBps_.end())
	{
		del_bpt(ea);
		sehHandlerBps_.erase(sehCit);
		if (config.getLogSEH())
		{
			std::ostringstream oss;
			oss << "IDAStealth: debugger reached top-level SEH handler at 0x" << std::hex << ea << "\n";
			logger_.log(oss.str());
			if (!config.getHaltInSEH()) continue_process();
		}
		if (config.getHaltInSEH())
		{
			logger_.log("IDAStealth: debugger has been halted in top-level SEH handler\n");
		}
	}
	else if (postSEHCit != postSEHBps_.end())
	{		
		del_bpt(ea);
		postSEHBps_.erase(postSEHCit);
		if (config.getLogSEH())
		{
			std::ostringstream oss;
			oss << "IDAStealth: debugger reached new location after SEH handler (possibly) modified EIP at 0x" << std::hex << ea << "\n";
			logger_.log(oss.str());
			if (!config.getHaltAfterSEH()) continue_process();
		}
		if (config.getHaltAfterSEH())
		{
			logger_.log("IDAStealth: debugger has been halted at instruction pointer after it was (possibly) modified by SEH handler\n");
		}
	}
}

uintptr_t IDAStealth::LocalStealthSession::getRtlDispatchExceptionAddr() const
{
	if (rtlDispatchExceptionAddr_) return rtlDispatchExceptionAddr_;

	// we need to disassemble the beginning of KiUserExceptionDispatcher to get RtlDispatchException
	_DecodedInst instructions[20];
	unsigned int instructionCount = 0;
	HMODULE hNtDll = GetModuleHandle("ntdll.dll");
	unsigned char* codePtr = (unsigned char*)GetProcAddress(hNtDll, "KiUserExceptionDispatcher");
	if (distorm_decode(0, codePtr, 20, Decode32Bits, instructions, 20, &instructionCount) == DECRES_SUCCESS)
	{
		for (int i=0; i<(int)instructionCount; ++i)
		{
			if (_stricmp((const char*)instructions[i].mnemonic.p, "call") == 0)
			{
				uintptr_t callOffset = 0;
				sscanf_s((const char*)instructions[i].operands.p, "%X", &callOffset);
				rtlDispatchExceptionAddr_ = (uintptr_t)codePtr + callOffset;
				return rtlDispatchExceptionAddr_;
			}
		}
	}
	throw std::runtime_error("Unable to locate RtlDispatchException in KiUserExceptionDispatcher");
}

// returns the address of the call to NtContinue inside KiUserExceptionDispatcher
uintptr_t IDAStealth::LocalStealthSession::getNtContinueCallAddr() const
{
	if (ntContinueCallAddr_) return ntContinueCallAddr_;

	_DecodedInst instructions[25];
	unsigned int instructionCount = 0;
	HMODULE hNtDll = GetModuleHandle("ntdll.dll");
	unsigned char* codePtr = (unsigned char*)GetProcAddress(hNtDll, "KiUserExceptionDispatcher");
	uintptr_t ntContinueAddr = (uintptr_t)GetProcAddress(hNtDll, "NtContinue");
	_DecodeResult result = distorm_decode(0, codePtr, 40, Decode32Bits, instructions, 25, &instructionCount);
	if (result == DECRES_SUCCESS || result == DECRES_MEMORYERR)
	{
		for (int i=0; i<(int)instructionCount; ++i)
		{
			if (_stricmp((const char*)instructions[i].mnemonic.p, "call") == 0)
			{
				uintptr_t callOffset = 0;
				sscanf_s((const char*)instructions[i].operands.p, "%X", &callOffset);
				uintptr_t callDestination = (uintptr_t)codePtr + callOffset;
				if (callDestination == ntContinueAddr)
				{
					ntContinueCallAddr_ = (uintptr_t)codePtr + (uintptr_t)instructions[i].offset;
					return ntContinueCallAddr_;
				}
			}
		}
	}
	throw std::runtime_error("Unable to locate call to NtContinue in KiUserExceptionDispatcher");
}

// init mechanism controlled by settings: stop at SEH handler / stop at EIP after SEH
void IDAStealth::LocalStealthSession::initSEHMonitoring() const
{
	const HideDebuggerConfig& config = HideDebuggerConfig::getInstance();
	if (config.getHaltInSEH() || config.getLogSEH()) setBP(getRtlDispatchExceptionAddr());
	if (config.getHaltAfterSEH() || config.getLogSEH()) setBP(getNtContinueCallAddr());
}

void IDAStealth::LocalStealthSession::cleanupSEHMonitoring() const
{
	const HideDebuggerConfig& config = HideDebuggerConfig::getInstance();
	if (config.getHaltInSEH() || config.getLogSEH()) del_bpt(getRtlDispatchExceptionAddr());
	if (config.getHaltAfterSEH() || config.getLogSEH()) del_bpt(getNtContinueCallAddr());
}

bool IDAStealth::LocalStealthSession::setBP(ea_t ea) const
{
	if (exist_bpt(ea)) return true;
	else return add_bpt(ea);
}