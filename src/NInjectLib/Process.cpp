#include "Process.h"
#include <iostream>
#include <sstream>

using namespace std;

Process::Process(DWORD processID) :
	hThread_(INVALID_HANDLE_VALUE),
	hProcess_(INVALID_HANDLE_VALUE),
	processID_(processID)
{
	hProcess_ = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, processID);
	if (hProcess_ == NULL)
	{
		DWORD lastErr = GetLastError();
		stringstream ss;
		ss << "Failed to get appropriate process access rights for process id: " << processID
		   << ", system error code: " << lastErr;
		throw ProcessHandleException(ss.str());
	}
}

Process::Process(const Process& instance)
{
	this->hProcess_ = this->hThread_ = INVALID_HANDLE_VALUE;
	if (!duplicateHandle(instance.hProcess_, &this->hProcess_)
		|| !duplicateHandle(instance.hThread_, &this->hThread_)) throw ProcessHandleException("Failed to duplicate handle!");
	this->processID_ = instance.processID_;
}

Process& Process::operator=(const Process& instance)
{
	if (!duplicateHandle(instance.hProcess_, &this->hProcess_)
		|| !duplicateHandle(instance.hThread_, &this->hThread_)) throw ProcessHandleException("Failed to duplicate handle!");
	this->processID_ = instance.processID_;
	return *this;
}

Process::~Process()
{
	if (hProcess_ != INVALID_HANDLE_VALUE) CloseHandle(hProcess_);
	if (hThread_ != INVALID_HANDLE_VALUE) CloseHandle(hThread_);
}

bool Process::duplicateHandle(HANDLE hSrc, HANDLE* hDest)
{
	if (hSrc == INVALID_HANDLE_VALUE) return true;
	return (DuplicateHandle(GetCurrentProcess(), 
						   hSrc, 
						   GetCurrentProcess(), 
						   hDest,
						   0,
						   FALSE,
						   DUPLICATE_SAME_ACCESS) == TRUE ? true : false);
}

void Process::writeMemory(LPVOID address, LPCVOID data, DWORD size) const
{
	SIZE_T written = 0;
	WriteProcessMemory(hProcess_, address, data, size, &written);
	if (written != size) throw MemoryAccessException("Write memory failed!");
}

void Process::readMemory(LPVOID address, LPVOID buffer, DWORD size) const
{
	SIZE_T read = 0;
	ReadProcessMemory(hProcess_, address, buffer, size, &read);
	if (read != size) throw MemoryAccessException("Read memory failed!");
}

MEMORY_BASIC_INFORMATION Process::queryMemory(LPVOID address) const
{
	MEMORY_BASIC_INFORMATION mbi;
	SIZE_T retVal = VirtualQueryEx(hProcess_, address, &mbi, sizeof(MEMORY_BASIC_INFORMATION));
	if (retVal == 0) throw MemoryQueryException("Unable to query memory");
	return mbi;
}

DWORD Process::protectMemory(LPVOID address, SIZE_T size, DWORD protect) const
{
	DWORD oldProtect;
	BOOL retVal = VirtualProtectEx(hProcess_, address, size, protect, &oldProtect);
	if (retVal == FALSE) throw MemoryProtectException("Unable to set memory protection", address);
	return oldProtect;
}

bool Process::startThread(LPVOID address, LPVOID param)
{
	hThread_ = CreateRemoteThread(hProcess_, NULL, 0, (LPTHREAD_START_ROUTINE)address, param, 0, NULL);
	if (hThread_ != INVALID_HANDLE_VALUE) SetThreadPriority(hThread_, THREAD_PRIORITY_TIME_CRITICAL);
	return (hThread_ != NULL);
}

// wait for remote thread to exit and close its handle
void Process::waitForThread()
{
	if (hThread_ == NULL) throw std::runtime_error("Invalid thread handle");
	WaitForSingleObject(hThread_, INFINITE);
	CloseHandle(hThread_);
	hThread_ = NULL;
}

LPVOID Process::allocMem(DWORD size) const
{
	return allocMem(size, MEM_RESERVE | MEM_COMMIT);	
}

LPVOID Process::allocMem(DWORD size, DWORD allocationType) const
{
	return allocMem(size, NULL, allocationType);
}

LPVOID Process::allocMem(DWORD size, LPVOID desiredAddress, DWORD allocationType) const
{
	LPVOID addr = VirtualAllocEx(hProcess_, desiredAddress, size, allocationType, PAGE_EXECUTE_READWRITE);
	if (addr == NULL) throw MemoryAllocationException("Failed to allocate memory");
	return addr;
}

bool Process::freeMem(LPVOID address) const
{
	return (VirtualFreeEx(hProcess_, address, 0, MEM_RELEASE) != 0);
}

// note: does not work in process start event
std::vector<MODULEENTRY32> Process::getModules() const
{
	std::vector<MODULEENTRY32> result;
	HANDLE hModulesSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processID_);
	if (hModulesSnap == INVALID_HANDLE_VALUE)
	{
		DWORD err = GetLastError();
		ostringstream oss;
		oss << "Unable to create modules snapshot, last error: " << err << std::endl;
		throw std::runtime_error(oss.str());
	}

	MODULEENTRY32 me32;
	me32.dwSize = sizeof(MODULEENTRY32);
	if(!Module32First(hModulesSnap, &me32))
	{
		DWORD err = GetLastError();
		ostringstream oss;
		oss << "Unable to enumerate modules from snapshot, last error: " << err << std::endl;
		CloseHandle(hModulesSnap);
		throw std::runtime_error(oss.str());
	}

	bool found = false;
	do
	{
		result.push_back(me32);
	} while(!found && Module32Next(hModulesSnap, &me32));

	CloseHandle(hModulesSnap);
	return result;
}

void Process::throwSysError(const char* msg, DWORD lastError) const
{
	std::ostringstream oss;
	oss << msg << ", system error was: " << lastError;
	throw std::runtime_error(oss.str());
}

// also works if process is suspended and not fully initialized yet
uintptr_t Process::getImageBase(HANDLE hThread) const
{
	CONTEXT context;
	context.ContextFlags = CONTEXT_SEGMENTS;
	if (!GetThreadContext(hThread, &context))
	{
		throwSysError("Error while retrieving thread context to determine IBA",  GetLastError());
	}

	// translate FS selector to virtual address
	LDT_ENTRY ldtEntry;
	if (!GetThreadSelectorEntry(hThread, context.SegFs, &ldtEntry))
	{
		throwSysError("Error while translating FS selector to virtual address",  GetLastError());
	}

	uintptr_t fsVA = (ldtEntry.HighWord.Bytes.BaseHi) << 24
		| (ldtEntry.HighWord.Bytes.BaseMid) << 16 | (ldtEntry.BaseLow);

	uintptr_t iba = 0;
	SIZE_T read;
	// finally read image based address from PEB:[8]
	if (!(ReadProcessMemory(hProcess_, (LPCVOID)(fsVA+0x30), &iba, sizeof(uintptr_t), &read)
		&& ReadProcessMemory(hProcess_, (LPCVOID)(iba+8), &iba, sizeof(uintptr_t), &read)))
	{
		throwSysError("Error while reading process memory to retrieve image base address", GetLastError());
	}
	return iba;
}

// retrieve image base address for current process
// ONLY works if process has bee initialized, otherwise thread enumeration will fail
// use overloaded function instead
uintptr_t Process::getImageBase() const
{
	// first get handle to one of the threads in the process
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, processID_);
	if (hSnapshot == INVALID_HANDLE_VALUE) throw std::runtime_error("Unable to create thread snapshot");

	DWORD lastError = 0;
	THREADENTRY32 threadEntry;
	threadEntry.dwSize = sizeof(THREADENTRY32);
	if (!Thread32First(hSnapshot, &threadEntry))
	{
		lastError = GetLastError();
		CloseHandle(hSnapshot);
		throwSysError("Unable to get first thread from snapshot", lastError);
	}

	CloseHandle(hSnapshot);
	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, threadEntry.th32ThreadID);
	if (!hThread)
	{
		lastError = GetLastError();
		throwSysError("Error while retrieving thread handle", lastError);
	}

	try
	{
		uintptr_t iba = getImageBase(hThread);
		CloseHandle(hThread);
		return iba;
	}
	catch (const std::exception&)
	{
		CloseHandle(hThread);
		throw;
	}
}