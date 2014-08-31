#include "HookHelper.h"
#include "IPCConfigExchangeReader.h"
#include <string>

// global variables
NtQueryInformationProcessFPtr origNtQueryInformationProcess = NULL;
NtSetInformationThreadFPtr origNtSetInformationThread = NULL;
NtQueryObjectFPtr origNtQueryObject = NULL;
NtCloseFPtr origNtClose = NULL;
NtQuerySystemInformationFPtr origNtQuerySystemInformation = NULL;
OutputDebugStringAFPtr origOutputDebugStringA = NULL;
OutputDebugStringWFPtr origOutputDebugStringW = NULL;
OpenProcessFPtr origOpenProcess = NULL;
KiUserExceptionDispatcherFPtr origKiUserExceptDisp = NULL;
NtSetContextThreadFPtr origSetThreadContext = NULL;
NtGetContextThreadFPtr origGetThreadContext = NULL;
NtYieldExecutionFPtr origNtYieldExecution = NULL;
FindWindowAFPtr origFindWindowA = NULL;
FindWindowWFPtr origFindWindowW = NULL;
FindWindowExAFPtr origFindWindowExA = NULL;
FindWindowExWFPtr origFindWindowExW = NULL;
EnumWindowsFPtr origEnumWindows = NULL;
NCodeHookIA32 nCodeHook;
const char* NTDLL = "ntdll.dll";
const char* K32DLL = "kernel32.dll";
const char* U32DLL = "user32.dll";

void dbgPrint(const std::string& msg)
{
	std::string dbgStr = "HideDebugger.dll: " + msg;
	OutputDebugStringA(dbgStr.c_str());
}

// convert given process handle to process ID
DWORD handleToProcessID(HANDLE hProcess)
{	
	PROCESS_BASIC_INFORMATION pbi;
	ZeroMemory(&pbi, sizeof(PROCESS_BASIC_INFORMATION));
	if (origNtQueryInformationProcess(hProcess, ProcessBasicInformation, &pbi, sizeof(PROCESS_BASIC_INFORMATION), NULL) == STATUS_SUCCESS)
		return pbi.UniqueProcessId;
	else return (DWORD)-1;
}

// return name of current process
std::wstring getProcessName()
{
	wchar_t pathBuffer[MAX_PATH];
	DWORD size = GetModuleFileNameW(NULL, pathBuffer, MAX_PATH - 1);
	if (!size) return std::wstring(L"");
	std::wstring exeName = std::wstring(pathBuffer);
	if (exeName[exeName.length()-1] == '\\') exeName.erase(exeName.length()-1);
	size_t index = exeName.find_last_of(L"\\") + 1;
	exeName.erase(exeName.begin(), exeName.begin() + index);
	return exeName;
}

// try to check if a given handle is valid
bool isHandleValid(HANDLE handle)
{
	// Note: we might produce false negatives here due to access restrictions of the debuggee (PROCESS_DUP_HANDLE)
	HANDLE duplicatedHandle;
	if (DuplicateHandle(GetCurrentProcess(), handle, GetCurrentProcess(), &duplicatedHandle, 0, FALSE, DUPLICATE_SAME_ACCESS))
	{
		CloseHandle(duplicatedHandle);
		return true;
	}
	return false;
}

void restoreNTHeaders()
{
	try
	{
		IPC::IPCConfigExchangeReader ipcReader;
		// we don't need to restore the PE headers when we attach to a process
		if (ipcReader.isPERestoreRequired())
		{
			IPC::IPCPEHeaderData peData = ipcReader.getIPCPEHeaderData();
			// read DOS header, advance to NT headers and restore them
			DWORD oldProtection;
			PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)peData.imageBase;
			VirtualProtect((LPVOID)pDosHeader, sizeof(IMAGE_DOS_HEADER), PAGE_EXECUTE_READWRITE, &oldProtection);
			PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)(pDosHeader->e_lfanew + peData.imageBase);
			VirtualProtect((LPVOID)pDosHeader, sizeof(IMAGE_DOS_HEADER), oldProtection, &oldProtection);

			VirtualProtect(pNtHeaders, sizeof(IMAGE_NT_HEADERS), PAGE_EXECUTE_READWRITE, &oldProtection);
			*pNtHeaders = peData.ntHeaders;
			VirtualProtect(pNtHeaders, sizeof(IMAGE_NT_HEADERS), oldProtection, &oldProtection);
		}
	}
	catch (const std::exception& e)
	{
		dbgPrint("Error while restoring NT headers: " + std::string(e.what()));
	}
	catch (...)
	{
		dbgPrint("SEH exception while trying to restore PE header");
	}
}

DWORD handleToThreadID(HANDLE hThread)
{
	typedef NTSTATUS (NTAPI *NtQIT)(HANDLE, THREAD_INFORMATION_CLASS, PVOID, ULONG, PULONG);
	HMODULE hNt = LoadLibrary(NTDLL);
	NtQIT pNtQueryInfThread = (NtQIT)GetProcAddress(hNt, "NtQueryInformationThread");
	if (pNtQueryInfThread)
	{
		THREAD_BASIC_INFORMATION tbi = {0};
		ULONG retLength;
		NTSTATUS status;
		status = pNtQueryInfThread(hThread, ThreadBasicInformation, &tbi, sizeof(THREAD_BASIC_INFORMATION), &retLength);
		if (status != STATUS_SUCCESS)
		{
			dbgPrint("Failed to translate thread handle to corresponding thread ID");
		}

		FreeLibrary(hNt);
		return (DWORD)tbi.ClientId.UniqueThread;
	}
	
	FreeLibrary(hNt);
	return 0;
}