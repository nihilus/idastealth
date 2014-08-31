#pragma once

#include <iostream>
#include <NCodeHook/NCodeHookInstantiation.h>
#include "ntdll.h"

// forward declarations

void dbgPrint(const std::string& msg);
DWORD handleToProcessID(HANDLE hProcess);
DWORD handleToThreadID(HANDLE hThread);
std::wstring getProcessName();
void restoreNTHeaders();
bool isHandleValid(HANDLE handle);

// original function pointers

extern NtQueryInformationProcessFPtr origNtQueryInformationProcess;
extern NtSetInformationThreadFPtr origNtSetInformationThread;
extern NtQueryObjectFPtr origNtQueryObject;
extern NtCloseFPtr origNtClose;
extern NtQuerySystemInformationFPtr origNtQuerySystemInformation;
extern OutputDebugStringAFPtr origOutputDebugStringA;
extern OutputDebugStringWFPtr origOutputDebugStringW;
extern OpenProcessFPtr origOpenProcess;
extern KiUserExceptionDispatcherFPtr origKiUserExceptDisp;
extern NtSetContextThreadFPtr origSetThreadContext;
extern NtGetContextThreadFPtr origGetThreadContext;
extern NtYieldExecutionFPtr origNtYieldExecution;
extern FindWindowAFPtr origFindWindowA;
extern FindWindowWFPtr origFindWindowW;
extern FindWindowExAFPtr origFindWindowExA;
extern FindWindowExWFPtr origFindWindowExW;
extern EnumWindowsFPtr origEnumWindows;

extern const char* NTDLL;
extern const char* K32DLL;
extern const char* U32DLL;

extern NCodeHookIA32 nCodeHook;