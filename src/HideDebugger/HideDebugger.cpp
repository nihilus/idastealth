// HideDebugger.cpp : Defines the entry point for the DLL application.
//

#include <boost/filesystem/operations.hpp>
#include "HideDebugger.h"
#include "HideDebuggerConfig.h"
#include "HookHelper.h"
#include <IniFileAccess/IniFileAccess.h>
#include <iostream>
#include "IPCConfigExchangeReader.h"
#include <map>
#include <Tlhelp32.h>

using namespace std;

void hideDebugger();
void applyConfigFromFile(const string& configFile, const string& profile);
void initFuncAddresses();

static bool xpOrLater_ = false;
static bool vistaOrLater_ = false;
static bool ntQuerySysInfo_ = false;
static bool fakeParentProcess_ = false;
static bool hideIDAProcess_ = false;
static unsigned int tickDelta_ = 23;

static uintptr_t ZwRaiseExceptAddr = 0;
static uintptr_t RtlDispatchExceptAddr = 0;
static uintptr_t ZwContinueAddr = 0;
static uintptr_t RtlRaiseExceptAddr = 0;

typedef struct 
{
	DWORD   Dr0;
	DWORD   Dr1;
	DWORD   Dr2;
	DWORD   Dr3;
	DWORD   Dr6;
	DWORD   Dr7;
} DEBUG_REGISTERS;

// critical section for map access from KiUserExceptionDispatcher
static CRITICAL_SECTION section_;
// contains contexts made of fake DRs and real values for all other registers
map<DWORD, CONTEXT> fakeContexts_;
// only contains the real DRs
map<DWORD, DEBUG_REGISTERS> realDRs_;

BOOL APIENTRY DllMain(HMODULE /*hModule*/, DWORD ul_reason_for_call, LPVOID /*lpReserved*/)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		restoreNTHeaders();
		InitializeCriticalSection(&section_);
		initFuncAddresses();
		hideDebugger();
		break;

	case DLL_THREAD_DETACH:
		fakeContexts_.erase(GetCurrentThreadId());
		break;

	case DLL_PROCESS_DETACH:
		DeleteCriticalSection(&section_);
		break;
	}
    return TRUE;
}

void hideDebugger()
{
	try
	{
		IPC::IPCConfigExchangeReader configExchange;
		string configFile = configExchange.getConfigFile();
		string profile = configExchange.getProfile();
		applyConfigFromFile(configFile, profile);
		//remoteEvents_ = RemoteEvent::RemoteEventWriterPtr(new RemoteEvent::RemoteEventWriter("", ""));
	}
	catch (const std::exception& e)
	{
		dbgPrint("Exception while trying to initialize stealth techniques: " + string(e.what()));
	}
}

// hook functions

NTSTATUS NTAPI NtQueryInformationProcessHook(HANDLE ProcessHandle, PROCESS_INFORMATION_CLASS ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength) 
{
	if (ProcessInformationClass == ProcessDebugPort || ProcessInformationClass == ProcessDebugFlags)
	{
		// if this process acts as a debugger, we will unveil ourselves if we hide the debug port
		// see also: http://forum.tuts4you.com/index.php?showtopic=16750
		if (handleToProcessID(ProcessHandle) != GetCurrentProcessId())
			return origNtQueryInformationProcess(ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength);
		
		DWORD procInfo;
		DWORD retLen;
		// try original API with the given handle, if it works we need to hide ourselves
		// otherwise original function will fail anyway
		if (origNtQueryInformationProcess(ProcessHandle, ProcessInformationClass, &procInfo, ProcessInformationLength, &retLen) != STATUS_SUCCESS)
			return origNtQueryInformationProcess(ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength);
		
		// handle is valid, so employ stealth!
		if (ReturnLength != NULL && !IsBadWritePtr(ReturnLength, sizeof(UINT_PTR)))
			*ReturnLength = 4;
		// hide debug port - if we cannot write result we will fail, so call original function
		if (IsBadWritePtr(ProcessInformation, sizeof(UINT_PTR)))
			return origNtQueryInformationProcess(ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength);
		else
		{
			if (ProcessInformationClass == ProcessDebugFlags) *((DWORD_PTR*)ProcessInformation) = 1;
			else *((DWORD_PTR*)ProcessInformation) = 0;
			return STATUS_SUCCESS;
		}
	}
	else if (ProcessInformationClass == ProcessDebugObjectHandle)
	{
		if (handleToProcessID(ProcessHandle) != GetCurrentProcessId())
			return origNtQueryInformationProcess(ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength);
		return STATUS_PORT_NOT_SET;
	}
	else if (ProcessInformationClass == ProcessBasicInformation && fakeParentProcess_)
	{
		// first we need to check if ProcessHandle is a handle to the current process
		// otherwise we need to skip this hook
		if (handleToProcessID(ProcessHandle) != GetCurrentProcessId())
			return origNtQueryInformationProcess(ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength);

		NTSTATUS retVal = origNtQueryInformationProcess(ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength);
		if (retVal == STATUS_SUCCESS)
		{
			DWORD processID = 0;
			GetWindowThreadProcessId(GetShellWindow(), &processID);
			DWORD* parentPID = (DWORD*)ProcessInformation;
			// patch parent process id of current process
			parentPID[5] = processID;
		}
		return retVal;
	}
	else return origNtQueryInformationProcess(ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength);
}

// hide debug object
static char* queryObjBuffer = NULL;
NTSTATUS NTAPI NtQueryObjectHook(HANDLE ObjectHandle, OBJECT_INFORMATION_CLASS ObjectInformationClass, PVOID ObjectInformation, ULONG Length, PULONG ResultLength)
{
	if (ObjectInformationClass == ObjectAllInformation && Length > 0)
	{
		NTSTATUS retVal = origNtQueryObject(ObjectHandle, ObjectInformationClass, ObjectInformation, Length, ResultLength);
		if (retVal != STATUS_SUCCESS) return retVal;

		unsigned int buffer = (unsigned int)ObjectInformation;
		ULONG count = *(ULONG*)buffer;
		PUNICODE_STRING pWStr = (PUNICODE_STRING)(buffer + sizeof(ULONG));
		unsigned int address = 0;
		
		// walk object structure and overwrite count for DebugObject with 0
		const WCHAR* dbgObj = L"DebugObject";
		const size_t objSize = sizeof(L"DebugObject") - 2;
		for (unsigned int i=0; i<count; ++i)
		{
			if (pWStr->Length == objSize)
			{
				// consider the case that object name is not zero-terminated
				if (memcmp(pWStr->Buffer, dbgObj, objSize) == 0)
				{
					// zero out handle and object count
					unsigned int* count = (unsigned int*)(&pWStr->Buffer) + 1;
					*count = 0; ++count;
					*count = 0;
				}
			}
			address = ((unsigned int)(pWStr->Buffer) + pWStr->Length) & -4;
			address += 4; // skip alignment bytes
			pWStr = (PUNICODE_STRING)address;
		}
		return retVal;
	}
	else return origNtQueryObject(ObjectHandle, ObjectInformationClass, ObjectInformation, Length, ResultLength);
}

// Protect the kernel callback KiRaiseUserExceptionDispatcher from being trashed by the debuggee
// because this function is executed in order to raise an exception whenever an invalid/protected
// handle is closed
NTSTATUS NTAPI NtCloseHook(HANDLE ObjectHandle)
{
	HMODULE hNtDll = LoadLibrary(NTDLL);
	unsigned char* ptr = (unsigned char*)GetProcAddress(hNtDll, "KiRaiseUserExceptionDispatcher");
	
	DWORD oldProtect;
	if (VirtualProtect(ptr, 1, PAGE_EXECUTE_READWRITE, &oldProtect))
	{
		// write ret instruction, so the kernel callback won't raise an exception in the first place
		*ptr = 0xC3;
		FlushInstructionCache(GetCurrentProcess(), ptr, 1);
		VirtualProtect(ptr, 1, oldProtect, &oldProtect);
	}
	
	FreeLibrary(hNtDll);
	return origNtClose(ObjectHandle);
}

// hide debugger by pretending there was no receiver for the debug string
// increase stealth by always returning 1
VOID WINAPI OutputDebugStringAHook(LPCSTR lpOutputString)
{
	origOutputDebugStringA(lpOutputString);
	SetLastError(ERROR_FILE_NOT_FOUND);
	__asm mov eax, 1
}

VOID WINAPI OutputDebugStringWHook(LPCWSTR lpOutputString)
{
	origOutputDebugStringW(lpOutputString);
	SetLastError(ERROR_FILE_NOT_FOUND);
	__asm mov eax, 1
}

// search in data returned from NtQuerySystemInformation for the corresponding
// chunk of the given process
PSYSTEM_PROCESS_INFORMATION findProcessChunk(PSYSTEM_PROCESS_INFORMATION pInfo, const wstring& processName)
{
	PSYSTEM_PROCESS_INFORMATION lastPInfo = pInfo;
	while (lastPInfo->NextEntryOffset)
	{
		lastPInfo = pInfo;
		LPCWSTR str = (LPCWSTR)pInfo->Reserved2[1];
		if (str)
			if (_wcsicmp(processName.c_str(), str) == 0) return pInfo;
		pInfo = (PSYSTEM_PROCESS_INFORMATION)((uintptr_t)pInfo + pInfo->NextEntryOffset);
	}
	return NULL;
}

// one hook for multiple stealth techniques
NTSTATUS NTAPI NtQuerySystemInformationHook(SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength)
{
	 // just query length?
	if (SystemInformationLength == 0) 
		return origNtQuerySystemInformation(SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength);

	if (ntQuerySysInfo_ && SystemInformationClass == SystemKernelDebuggerInformation)
	{
		NTSTATUS retVal = origNtQuerySystemInformation(SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength);
		if (retVal != STATUS_SUCCESS) return retVal;
		
		*(unsigned short*)SystemInformation = 0x0100;
		return STATUS_SUCCESS;
	}
	
	if (SystemInformationClass == SystemProcessInformation)
	{
		// get original process list		
		NTSTATUS retVal = origNtQuerySystemInformation(SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength);
		if (retVal != STATUS_SUCCESS) return retVal;

		// parameters are ok and we have the process list, now apply stealth techniques
		if (hideIDAProcess_)
		{
			PSYSTEM_PROCESS_INFORMATION pInfo = (PSYSTEM_PROCESS_INFORMATION)SystemInformation;
			PSYSTEM_PROCESS_INFORMATION lastPInfo = pInfo;
			// we need to walk the whole list because there might be multiple instances
			while (pInfo->NextEntryOffset)
			{
				LPCWSTR processName = (LPCWSTR)pInfo->Reserved2[1];
				// increase size of last chunk so we just skip over the IDA chunk
				if (processName)
					if (_wcsicmp(processName, L"idag.exe") == 0 || _wcsicmp(processName, L"idaw.exe") == 0)
					{
						lastPInfo->NextEntryOffset += pInfo->NextEntryOffset; 
						size_t len = wcslen(processName) * sizeof(wchar_t);
						if (len) memset((void*)processName, 0, len);
					}
				lastPInfo = pInfo;
				pInfo = (PSYSTEM_PROCESS_INFORMATION)((uintptr_t)pInfo + pInfo->NextEntryOffset);
			}
		}

		if (fakeParentProcess_)
		{
			// first search process id of explorer
			PSYSTEM_PROCESS_INFORMATION explorerPInfo = findProcessChunk((PSYSTEM_PROCESS_INFORMATION)SystemInformation, L"explorer.exe");
			if (explorerPInfo)
			{
				PSYSTEM_PROCESS_INFORMATION ownPInfo = (PSYSTEM_PROCESS_INFORMATION)SystemInformation;
				wstring processName = getProcessName();
				// we need to walk the whole list because there might be multiple instances running
				while (ownPInfo->NextEntryOffset)
				{
					ownPInfo = findProcessChunk(ownPInfo, processName);
					// finally replace parent process id
					if (ownPInfo) ownPInfo->ParentProcessId = explorerPInfo->UniqueProcessId;
					else break;
					ownPInfo = (PSYSTEM_PROCESS_INFORMATION)((uintptr_t)ownPInfo + ownPInfo->NextEntryOffset);
				}
			}
		}
		return retVal;
	}

	// fall through for default processing
	return origNtQuerySystemInformation(SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength);
}

NTSTATUS NTAPI NtSetInformationThreadHook(HANDLE ThreadHandle, THREAD_INFORMATION_CLASS ThreadInformationClass, PVOID ThreadInformation, ULONG ThreadInformationLength)
{
	if (!isHandleValid(ThreadHandle)) return STATUS_INVALID_HANDLE;
	if (ThreadInformationClass == ThreadHideFromDebugger) return STATUS_SUCCESS;
	return origNtSetInformationThread(ThreadHandle, ThreadInformationClass, ThreadInformation, ThreadInformationLength);
}

bool isCSRSS(DWORD processID)
{
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if(hProcessSnap == INVALID_HANDLE_VALUE) return NULL;

	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if(!Process32First(hProcessSnap, &pe32))
	{
		CloseHandle( hProcessSnap );
		return NULL;
	}

	bool retVal = false;
	bool found = false;
	do
	{
		string exeFile = string(pe32.szExeFile);
		transform(exeFile.begin(), exeFile.end(), exeFile.begin(), tolower);
		if (exeFile.find("csrss.exe") != string::npos)
		{
			found = true;
			retVal = (processID == pe32.th32ProcessID) ? true : false;
		}
	} while(!found && Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	return retVal;
}

// prevent access to CSRSS process
HANDLE WINAPI OpenProcessHook(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId)
{
	if (isCSRSS(dwProcessId))
	{
		// set proper last error to increase stealth
		SetLastError(ERROR_ACCESS_DENIED);
		return NULL;
	}
	else return origOpenProcess(dwDesiredAccess, bInheritHandle, dwProcessId);
}

// just disable it
BOOL WINAPI SwitchDesktopHook(HDESK hDesktop)
{
	if (!isHandleValid(hDesktop)) return FALSE;
	return TRUE;
}

DWORD WINAPI GetTickCountHook()
{
	static bool tickCounterInitialized = false;
	static LARGE_INTEGER counter = { 23 };
	
	if (!tickCounterInitialized)
	{
		QueryPerformanceCounter(&counter);
		tickCounterInitialized = true;
	}
	else
	{
		if (tickDelta_ != 0) counter.QuadPart += rand() % tickDelta_;
	}

	DWORD retVal;
	// mimic original implementation to increase stealth
	__asm
	{
		lea		edx, counter
		mov		eax, [edx]
		mov		edx, [edx+4]
		mul		edx
		shrd	eax, edx, 24
		mov		retVal, eax
	}
	return retVal;
}

BOOL WINAPI BlockInputHook(BOOL /*fBlockIt*/)
{
	return TRUE;
}

DWORD WINAPI SuspendThreadHook(HANDLE hThread)
{
	if (!isHandleValid(hThread)) return (DWORD)-1;
	return 0;
}

// convert ascii string to widechar string
wstring strToWstr(const string& str)
{
	wstring wstr(str.length() + 1, 0);
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), &wstr[0], str.length());
	return wstr;
}

// check if given class name matches any of the IDA window classes
bool filterClassName(const wstring& name)
{
	static wchar_t* names[] = { L"idawindow", L"tnavbox", L"idaview", L"tgrzoom" };
	wstring tmp(name.length(), 0);
	transform(name.begin(), name.end(), tmp.begin(), tolower);
	for (int i=0; i<ARRAYSIZE(names); ++i) if (tmp.find(names[i]) != string::npos) return true;
	return false;
}

//check if given window name matches any of the IDA window captions
bool filterWndName(const wstring& name)
{
	static wchar_t* names[] = { L"ida", L"graph overview", L"idc scripts", L"disassembly", 
								L"program segmentation", L"call stack", L"general registers",
								L"breakpoint", L"structure offsets", L"database notepad", L"threads",
								L"segment translation", L"imports", L"desktopform", L"function calls",
								L"structures", L"strings window", L"functions window", L"no signature"};
	wstring tmp(name.length(), 0);
	transform(name.begin(), name.end(), tmp.begin(), tolower);
	for (int i=0; i<ARRAYSIZE(names); ++i) if (tmp.find(names[i]) != string::npos) return true;
	return false;
}

HWND WINAPI FindWindowAHook(LPCSTR lpClassName, LPCSTR lpWindowName)
{
	if (lpClassName != NULL)
		if (filterClassName(strToWstr(lpClassName))) return NULL;
	
	if (lpWindowName != NULL)
		if (filterWndName(strToWstr(lpWindowName))) return NULL;
	
	return origFindWindowA(lpClassName, lpWindowName);	
}

HWND WINAPI FindWindowWHook(LPCWSTR lpClassName, LPCWSTR lpWindowName)
{
	if (lpClassName != NULL)
		if (filterClassName(lpClassName)) return NULL;

	if (lpWindowName != NULL)
		if (filterWndName(lpWindowName)) return NULL;

	return origFindWindowW(lpClassName, lpWindowName);
}

HWND WINAPI FindWindowExAHook(HWND hWndParent, HWND hWndChildAfter, LPCSTR lpszClass, LPCSTR lpszWindow)
{
	if (lpszClass != NULL)
		if (filterClassName(strToWstr(lpszClass))) return NULL;

	if (lpszWindow != NULL)
		if (filterWndName(strToWstr(lpszWindow))) return NULL;

	return origFindWindowExA(hWndParent, hWndChildAfter, lpszClass, lpszWindow);
}

HWND WINAPI FindWindowExWHook(HWND hWndParent, HWND hWndChildAfter, LPCWSTR lpszClass, LPCWSTR lpszWindow)
{
	if (lpszClass != NULL)
		if (filterClassName(lpszClass)) return NULL;

	if (lpszWindow != NULL)
		if (filterWndName(lpszWindow)) return NULL;

	return origFindWindowExW(hWndParent, hWndChildAfter, lpszClass, lpszWindow);
}

static WNDENUMPROC enumCallback = NULL;
BOOL CALLBACK enumWndFilterCallback(HWND hwnd, LPARAM lParam)
{
	wchar_t tmp[666];
	if (GetWindowTextW(hwnd, tmp, 666))
	{
		wstring wndText(tmp);
		if (filterWndName(wndText)) return TRUE;
	}
	return enumCallback(hwnd, lParam);
}

BOOL WINAPI EnumWindowsHook(WNDENUMPROC lpEnumFunc, LPARAM lParam)
{
	enumCallback = lpEnumFunc;
	return origEnumWindows(enumWndFilterCallback, lParam);
}

// init some function addresses needed for the re-implementation of KiUserExceptionDispatcher
void initFuncAddresses()
{
	OSVERSIONINFO osVi;
	osVi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osVi);
	xpOrLater_ = (osVi.dwMajorVersion > 5) ||
		((osVi.dwMajorVersion == 5) && (osVi.dwMinorVersion >= 1));
	vistaOrLater_ = (osVi.dwMajorVersion > 5);

	HMODULE hNtDll = LoadLibrary(NTDLL);
	ZwRaiseExceptAddr = (uintptr_t)GetProcAddress(hNtDll, "ZwRaiseException");
	
	// we need to disassemble the beginning of KiUserExceptionDispatcher to get RtlDispatchException
	_DecodedInst instructions[20];
	unsigned int instructionCount = 0;
	unsigned char* codePtr = (unsigned char*)GetProcAddress(hNtDll, "KiUserExceptionDispatcher");
	distorm_decode(0, codePtr, 20, Decode32Bits, instructions, 20, &instructionCount);
	for (int i=0; i<20; ++i)
	{
		if (_stricmp((const char*)instructions[i].mnemonic.p, "call") == 0)
		{
			uintptr_t callOffset = 0;
			sscanf_s((const char*)instructions[i].operands.p, "%X", &callOffset);
			RtlDispatchExceptAddr = (uintptr_t)codePtr + callOffset;
			break;
		}
	}
	
	ZwContinueAddr = (uintptr_t)GetProcAddress(hNtDll, "ZwContinue");
	RtlRaiseExceptAddr = (uintptr_t)GetProcAddress(hNtDll, "RtlRaiseException");
	FreeLibrary(hNtDll);
}

// returns the fake context for the current thread
const CONTEXT* getEmulationDRs()
{
	DWORD id = GetCurrentThreadId();
	map<DWORD, CONTEXT>::const_iterator cit = fakeContexts_.find(id);
	if (cit != fakeContexts_.end()) return &cit->second;
	else
	{
		// if we cannot find an entry for the curren thread we create a new one
		CONTEXT newCtx;
		ZeroMemory(&newCtx, sizeof(CONTEXT));
		pair<map<DWORD, CONTEXT>::iterator, bool> p = fakeContexts_.insert(make_pair(id, newCtx));
		return &((p.first)->second);
	}
}

// preserve real DRs to map and overwrite DRs in supplied context with DRs from emulation storage
void handleDRs(PCONTEXT pContext)
{
	const CONTEXT* ctx = getEmulationDRs();

	DEBUG_REGISTERS realDRs;
	// save REAL debug registers (are likely in use by our debugger, so we need to preserve them!)
	memcpy(&realDRs, &pContext->Dr0, sizeof(DEBUG_REGISTERS));
	realDRs_.insert(make_pair(GetCurrentThreadId(), realDRs));
	// overwrite real DRs with our EMULATION!
	memcpy(&pContext->Dr0, &ctx->Dr0, sizeof(DEBUG_REGISTERS));
}

// merge original DRs back into context which was possibly modified by the exception handler
void mergeRegisters(PCONTEXT pOrigContext)
{
	map<DWORD, DEBUG_REGISTERS>::const_iterator cit = realDRs_.find(GetCurrentThreadId());
	if (cit != realDRs_.end())
	{
		const DEBUG_REGISTERS* src = &cit->second;
		memcpy(&pOrigContext->Dr0, src, sizeof(DEBUG_REGISTERS));
	}
}

void __declspec(naked) NTAPI KiUserExceptionDispatcherHook(PEXCEPTION_RECORD /*pExceptRec*/, PCONTEXT pContext)
{
	__asm
	{
		xor		eax, eax					// push fake address on stack so compiler generated code works!
		push	eax						
		push	ebp							// setup standard stack frame
		mov		ebp, esp
		sub		esp, __LOCAL_SIZE			// get required size from compiler
	}
	handleDRs(pContext);
	// now we have to call RtlDispatchException function and we need to be sure the stack is
	// in the same state as it would be without our hook because that function might never return!
	__asm
	{
		add		esp, __LOCAL_SIZE
		pop		ebp
		add		esp, 4						// remove fake address
		mov		ecx, [esp+4]
		mov		ebx, [esp+0]
		push	ecx
		push	ebx
		mov		eax, RtlDispatchExceptAddr
		call	eax							// stack is now clean, call RtlDispatchException
		pop		ebx
		pop		ecx
		or		al, al
		jz		RaiseExcept
		// merge modified context with preserved DRs
		push	ecx
		call	mergeRegisters
		pop		ecx							// PCONTEXT
		push	0
		push	ecx
		mov		eax, ZwContinueAddr
		call	eax
		// this code is probably never reached at al
		jmp RtlRaiseExcept

RaiseExcept:
		push	0
		push	ecx
		push	ebx
		call	ZwRaiseExceptAddr

		// original code from KiUserExceptionDispatcher
RtlRaiseExcept:
		add		esp, 0FFFFFFECh
		mov     [esp], eax
		mov     dword ptr [esp+4], 1
		mov     [esp+8], ebx
		mov     dword ptr [esp+10h], 0
		push    esp
		call    RtlRaiseExceptAddr
		retn    8
	}	
}

NTSTATUS NTAPI NtSetContextThreadHook(HANDLE hThread, const CONTEXT* lpContext)
{
	if (IsBadReadPtr(lpContext, sizeof(CONTEXT)) ||
		IsBadWritePtr((LPVOID)lpContext, sizeof(CONTEXT)))
	{
		// the original API could succeed if context is not writable (but readable)
		// however, we will fail in this case because we need to write the modified context flags
		return ERROR_NOACCESS;
	}
	else
	{
		EnterCriticalSection(&section_);
		DWORD id = handleToThreadID(hThread);

		// just save context to our map
		CONTEXT ctx;
		memcpy(&ctx, lpContext, sizeof(CONTEXT));
		fakeContexts_[id] = ctx;

		LeaveCriticalSection(&section_);
		
		// we don't permit modification of debug registers
		const_cast<LPCONTEXT>(lpContext)->ContextFlags = lpContext->ContextFlags & ~0x00000010;
		return origSetThreadContext(hThread, lpContext);
	}
}

NTSTATUS NTAPI NtGetContextThreadHook(HANDLE hThread, LPCONTEXT lpContext)
{
	EnterCriticalSection(&section_);

	DWORD id = handleToThreadID(hThread);
	map<DWORD, CONTEXT>::const_iterator cit = fakeContexts_.find(id);
	// if we didn't find an entry in our context map ask real API and add result to our storage
	if (cit == fakeContexts_.end()) 
	{
		NTSTATUS retVal = origGetThreadContext(hThread, lpContext);
		if (retVal == STATUS_SUCCESS)
		{
			CONTEXT ctx;
			memcpy(&ctx, lpContext, sizeof(CONTEXT));
			// in order to hide the debug registers from the app, we zero them out before returning
			ZeroMemory(&ctx.Dr0, 6*sizeof(DWORD));
			ZeroMemory(&lpContext->Dr0, 6*sizeof(DWORD));
			fakeContexts_[id] = ctx;
		}
		else
		{
			LeaveCriticalSection(&section_);
			return retVal;
		}
	}
	else
	{
		// return real context and possibly update with DRs from internal storage
		if (origGetThreadContext(hThread, lpContext) && lpContext->ContextFlags | CONTEXT_DEBUG_REGISTERS)
		{
			LPCONTEXT internalContext = (LPCONTEXT)&cit->second;
			lpContext->Dr0 = internalContext->Dr0;
			lpContext->Dr1 = internalContext->Dr1;
			lpContext->Dr2 = internalContext->Dr2;
			lpContext->Dr3 = internalContext->Dr3;
			lpContext->Dr6 = internalContext->Dr6;
			lpContext->Dr7 = internalContext->Dr7;
		}
	}

	LeaveCriticalSection(&section_);
	return STATUS_SUCCESS;
}

NTSTATUS NTAPI NtYieldExecutionHook()
{
	origNtYieldExecution();
	return 1;
}

NTSTATUS NTAPI RtlGetVersionHook(PRTL_OSVERSIONINFOW lpVersionInfo)
{
	// oddly enough, the original API behaves this way: if we specify an incorrect size
	// the function will still return successfully!
	// the high level wrappers are the only ones to check the supplied length, i.e. GetVersion(Ex)
	// to mimic the original API, we try to fill as much info into the buffer as we can
	// therefore we need to start at offset zero and continue until we are done or get an exception
	// Note: this behaviour seems to be very much OS specific, i.e. different on 2000, XP and Vista
	__try
	{
		if (lpVersionInfo->dwOSVersionInfoSize == sizeof(RTL_OSVERSIONINFOEXW))
		{
			PRTL_OSVERSIONINFOEXW viex = (PRTL_OSVERSIONINFOEXW)lpVersionInfo;
			viex->dwMajorVersion = 5;
			viex->dwMinorVersion = 1;
			viex->dwBuildNumber = 0xA28;
			viex->dwPlatformId = 2;
			wcscpy_s(viex->szCSDVersion, L"Service Pack 3");
			viex->wServicePackMajor = 3;
			viex->wServicePackMinor = 0;
			viex->wProductType = 1;
			viex->wSuiteMask = 0x100;
		}
		else 
		{
			// mimic the original API: we try to copy OSVERIOSNINFO to the supplied buffer
			lpVersionInfo->dwMajorVersion = 5;
			lpVersionInfo->dwMinorVersion = 1;
			lpVersionInfo->dwBuildNumber = 0xA28;
			lpVersionInfo->dwPlatformId = 2;
			wcscpy_s(lpVersionInfo->szCSDVersion, L"Service Pack 3");
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	}

	return STATUS_SUCCESS;
}

// set all version infos to WinXP SP3
DWORD WINAPI GetVersionHook()
{
	return 0x0a280105;
}

NTSTATUS NTAPI NtTerminateProcessHook(HANDLE ProcessHandle, NTSTATUS /*ExitStatus*/)
{
	if (!isHandleValid(ProcessHandle)) return STATUS_INVALID_HANDLE;
	return STATUS_SUCCESS;
}

NTSTATUS NTAPI NtTerminateThreadHook(HANDLE ThreadHandle, NTSTATUS /*ExitStatus*/)
{
	if (!isHandleValid(ThreadHandle)) return STATUS_INVALID_HANDLE;
	return STATUS_SUCCESS;
}

// end hook functions

void patchPEB()
{
	__asm
	{
		mov eax, fs:[18h]
		mov eax, [eax+30h]
		mov byte ptr [eax+2], 0
		
		//mov eax, fs:[30h]
		//mov byte ptr [eax+2], 0
	}
}

void patchNtGlobalFlag()
{
	unsigned char* ntGlobalFlag = NULL;
	__asm
	{
		mov eax, fs:[30h]
		lea eax, [eax+68h]
		mov ntGlobalFlag, eax
	}
	// remove only 0x70 bits, leave other flags intact
	*ntGlobalFlag &= ~0x70;
}

void patchHeapFlags()
{
	unsigned int* heapFlag = NULL;
	unsigned int* forceFlag = NULL;
	if (vistaOrLater_)
	{
		__asm
		{
			mov eax, fs:[30h]
			mov eax, [eax+18h]
			lea ecx, [eax+40h]
			mov heapFlag, ecx
			lea ecx, [eax+44h]
			mov forceFlag, ecx
		}
	}
	else
	{
		__asm
		{
			mov eax, fs:[30h]
			mov eax, [eax+18h]
			lea ecx, [eax+0Ch]
			mov heapFlag, ecx
			lea ecx, [eax+10h]
			mov forceFlag, ecx
		}
	}
	
	*heapFlag &= 2;
	*forceFlag = 0;
}

void applyConfigFromFile(const string& configFile, const string& profile)
{
	try
	{
		HideDebuggerConfig& config = HideDebuggerConfig::getInstance();
		config.setConfigFile(configFile);
		config.switchToProfile(profile);

		bool forceAbsJumps = config.getInlinePatching() == ForceAbsolute;
		nCodeHook.forceAbsoluteJumps(forceAbsJumps);
		
		if (config.getHeapFlags()) patchHeapFlags();
		if (config.getPEBIsDebugged()) patchPEB();
		if (config.getNtGlobalFlag()) patchNtGlobalFlag();
		ntQuerySysInfo_ = config.getNtQuerySystemInfo();
		fakeParentProcess_ = config.getFakeParentProcess();
		hideIDAProcess_ = config.getHideIDAProcess();
		if (ntQuerySysInfo_ || fakeParentProcess_ || hideIDAProcess_) 
			origNtQuerySystemInformation = nCodeHook.createHookByName(NTDLL, NtQuerySystemInfoStr, NtQuerySystemInformationHook);
		if (config.getNtQueryInfoProcess() || fakeParentProcess_)
			origNtQueryInformationProcess = nCodeHook.createHookByName(NTDLL, NtQueryInfoProcessStr, NtQueryInformationProcessHook);

		// only hook if OS >= XP
		if (config.getNtQueryObject() && xpOrLater_)
			origNtQueryObject = nCodeHook.createHookByName(NTDLL, NtQueryObjectStr, NtQueryObjectHook);
		if (config.getNtClose())
			origNtClose = nCodeHook.createHookByName(NTDLL, NtCloseStr, NtCloseHook);
		if (config.getOutputDbgStr())
		{
			origOutputDebugStringA = nCodeHook.createHookByName(K32DLL, "OutputDebugStringA", OutputDebugStringAHook);
			origOutputDebugStringW = nCodeHook.createHookByName(K32DLL, "OutputDebugStringW", OutputDebugStringWHook);
		}
		if (config.getNtSetInfoThread())
			origNtSetInformationThread = nCodeHook.createHookByName(NTDLL, NtSetInfoThreadStr, NtSetInformationThreadHook);
		if (config.getSuspendThread())
			nCodeHook.createHookByName(K32DLL, SuspendThreadStr, SuspendThreadHook);
		if (config.getGetTickCount())
		{
			nCodeHook.createHookByName(K32DLL, GetTickCountStr, GetTickCountHook);
			tickDelta_ = config.getGetTickCountDelta();
		}
		if (config.getBlockInput())
			nCodeHook.createHookByName(U32DLL, BlockInputStr, BlockInputHook);
		if (config.getOpenProcess())
			origOpenProcess = nCodeHook.createHook(OpenProcess, OpenProcessHook);
		if (config.getSwitchDesktop()) nCodeHook.createHook(SwitchDesktop, SwitchDesktopHook);
		if (config.getProtectDRs())
		{
			origKiUserExceptDisp = nCodeHook.createHookByName(NTDLL, "KiUserExceptionDispatcher", KiUserExceptionDispatcherHook);
			origSetThreadContext = nCodeHook.createHookByName(NTDLL, "NtSetContextThread", NtSetContextThreadHook);
			origGetThreadContext = nCodeHook.createHookByName(NTDLL, "NtGetContextThread", NtGetContextThreadHook);
		}
		if (config.getNtYieldExecution())
			origNtYieldExecution = nCodeHook.createHookByName(NTDLL, NtYieldExecutionStr, NtYieldExecutionHook);
		if (config.getHideIDAWindow())
		{
			origFindWindowA = nCodeHook.createHookByName(U32DLL, "FindWindowA", FindWindowAHook);
			origFindWindowExA = nCodeHook.createHookByName(U32DLL, "FindWindowExA", FindWindowExAHook);
			origFindWindowW = nCodeHook.createHookByName(U32DLL, "FindWindowW", FindWindowWHook);
			origFindWindowExW = nCodeHook.createHookByName(U32DLL, "FindWindowExW", FindWindowExWHook);
			origEnumWindows = nCodeHook.createHookByName(U32DLL, "EnumWindows", EnumWindowsHook);
		}
		if (config.getNtTerminate())
		{
			nCodeHook.createHookByName(NTDLL, "NtTerminateThread", NtTerminateThreadHook);
			nCodeHook.createHookByName(NTDLL, "NtTerminateProcess", NtTerminateProcessHook);
		}
		if (config.getGetVersion())
		{
			nCodeHook.createHookByName(K32DLL, "GetVersion", GetVersionHook);
			nCodeHook.createHookByName(NTDLL, "RtlGetVersion", RtlGetVersionHook);
		}
	}
	catch (const runtime_error& e)
	{
		dbgPrint("Exception while trying to create stealth hooks: " + string(e.what()));
	}
	catch (...)
	{
		dbgPrint("Unknown exception while creating stealth hooks");
	}
}