#pragma once

#include <Windows.h>

#pragma warning (push)

#pragma warning(disable : 4005)
#include <ntstatus.h>

#pragma warning (pop)

typedef LONG NTSTATUS;

typedef enum _OBJECT_INFORMATION_CLASS {

	ObjectBasicInformation, 
	ObjectNameInformation, 
	ObjectTypeInformation, 
	ObjectAllInformation, 
	ObjectDataInformation

} OBJECT_INFORMATION_CLASS, *POBJECT_INFORMATION_CLASS;

typedef enum _PROCESS_INFORMATION_CLASS {

	ProcessBasicInformation,
	ProcessQuotaLimits,
	ProcessIoCounters,
	ProcessVmCounters,
	ProcessTimes,
	ProcessBasePriority,
	ProcessRaisePriority,
	ProcessDebugPort,
	ProcessExceptionPort,
	ProcessAccessToken,
	ProcessLdtInformation,
	ProcessLdtSize,
	ProcessDefaultHardErrorMode,
	ProcessIoPortHandlers,          // Note: this is kernel mode only
	ProcessPooledUsageAndLimits,
	ProcessWorkingSetWatch,
	ProcessUserModeIOPL,
	ProcessEnableAlignmentFaultFixup,
	ProcessPriorityClass,
	ProcessWx86Information,
	ProcessHandleCount,
	ProcessAffinityMask,
	ProcessPriorityBoost,
	ProcessDeviceMap,
	ProcessSessionInformation,
	ProcessForegroundInformation,
	ProcessWow64Information,
	ProcessImageFileName,
	ProcessLUIDDeviceMapsEnabled,
	ProcessBreakOnTermination,
	ProcessDebugObjectHandle,
	ProcessDebugFlags,
	ProcessHandleTracing,
	ProcessIoPriority,
	ProcessExecuteFlags,
	ProcessTlsInformation,
	ProcessCookie,
	ProcessImageInformation,
	ProcessCycleTime,
	ProcessPagePriority,
	ProcessInstrumentationCallback,
	ProcessThreadStackAllocation,
	ProcessWorkingSetWatchEx,
	ProcessImageFileNameWin32,
	ProcessImageFileMapping,
	ProcessAffinityUpdateMode,
	ProcessMemoryAllocationMode,
	ProcessGroupInformation,
	ProcessTokenVirtualizationEnabled,
	ProcessConsoleHostProcess,
	ProcessWindowInformation,
	MaxProcessInfoClass             // MaxProcessInfoClass should always be the last enum

} PROCESS_INFORMATION_CLASS, *PPROCESS_INFORMATION_CLASS;

typedef struct _PEB {
	BYTE Reserved1[2];
	BYTE BeingDebugged;
	BYTE Reserved2[229];
	PVOID Reserved3[59];
	ULONG SessionId;
} PEB, *PPEB;

typedef struct _PROCESS_BASIC_INFORMATION {
	PVOID Reserved1;
	PPEB PebBaseAddress;
	PVOID Reserved2[2];
	ULONG_PTR UniqueProcessId;
	PVOID Reserved3;
} PROCESS_BASIC_INFORMATION;

typedef enum _THREAD_INFORMATION_CLASS {

	ThreadBasicInformation, 
	ThreadTimes, 
	ThreadPriority, 
	ThreadBasePriority, 
	ThreadAffinityMask, 
	ThreadImpersonationToken, 
	ThreadDescriptorTableEntry, 
	ThreadEnableAlignmentFaultFixup, 
	ThreadEventPair, 
	ThreadQuerySetWin32StartAddress, 
	ThreadZeroTlsCell, 
	ThreadPerformanceCount, 
	ThreadAmILastThread, 
	ThreadIdealProcessor, 
	ThreadPriorityBoost, 
	ThreadSetTlsArrayAddress, 
	ThreadIsIoPending, 
	ThreadHideFromDebugger

} THREAD_INFORMATION_CLASS, *PTHREAD_INFORMATION_CLASS;

typedef enum _SYSTEM_INFORMATION_CLASS {
	SystemBasicInformation = 0,
	SystemPerformanceInformation = 2,
	SystemTimeOfDayInformation = 3,
	SystemProcessInformation = 5,
	SystemProcessorPerformanceInformation = 8,
	SystemInterruptInformation = 23,
	SystemExceptionInformation = 33,
	SystemKernelDebuggerInformation = 35,
	SystemRegistryQuotaInformation = 37,
	SystemLookasideInformation = 45
} SYSTEM_INFORMATION_CLASS;

typedef struct _UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;

typedef enum _DBG_STATE
{
	DbgIdle,
	DbgReplyPending,
	DbgCreateThreadStateChange,
	DbgCreateProcessStateChange,
	DbgExitThreadStateChange,
	DbgExitProcessStateChange,
	DbgExceptionStateChange,
	DbgBreakpointStateChange,
	DbgSingleStepStateChange,
	DbgLoadDllStateChange,
	DbgUnloadDllStateChange
} DBG_STATE, *PDBG_STATE;

typedef struct _CLIENT_ID {
	HANDLE UniqueProcess;
	HANDLE UniqueThread;
} CLIENT_ID;
typedef CLIENT_ID *PCLIENT_ID;

typedef struct _DBGKM_EXCEPTION
{
	EXCEPTION_RECORD ExceptionRecord;
	ULONG FirstChance;
} DBGKM_EXCEPTION, *PDBGKM_EXCEPTION;

typedef struct _DBGKM_CREATE_THREAD
{
	ULONG SubSystemKey;
	PVOID StartAddress;
} DBGKM_CREATE_THREAD, *PDBGKM_CREATE_THREAD;

typedef struct _DBGKM_CREATE_PROCESS
{
	ULONG SubSystemKey;
	HANDLE FileHandle;
	PVOID BaseOfImage;
	ULONG DebugInfoFileOffset;
	ULONG DebugInfoSize;
	DBGKM_CREATE_THREAD InitialThread;
} DBGKM_CREATE_PROCESS, *PDBGKM_CREATE_PROCESS;

typedef struct _DBGKM_EXIT_THREAD
{
	NTSTATUS ExitStatus;
} DBGKM_EXIT_THREAD, *PDBGKM_EXIT_THREAD;

typedef struct _DBGKM_EXIT_PROCESS
{
	NTSTATUS ExitStatus;
} DBGKM_EXIT_PROCESS, *PDBGKM_EXIT_PROCESS;

typedef struct _DBGKM_LOAD_DLL
{
	HANDLE FileHandle;
	PVOID BaseOfDll;
	ULONG DebugInfoFileOffset;
	ULONG DebugInfoSize;
	PVOID NamePointer;
} DBGKM_LOAD_DLL, *PDBGKM_LOAD_DLL;

typedef struct _DBGKM_UNLOAD_DLL
{
	PVOID BaseAddress;
} DBGKM_UNLOAD_DLL, *PDBGKM_UNLOAD_DLL;

typedef struct _DBGUI_WAIT_STATE_CHANGE
{
	DBG_STATE NewState;
	CLIENT_ID AppClientId;
	union
	{
		struct
		{
			HANDLE HandleToThread;
			DBGKM_CREATE_THREAD NewThread;
		} CreateThread;
		struct
		{
			HANDLE HandleToProcess;
			HANDLE HandleToThread;
			DBGKM_CREATE_PROCESS NewProcess;
		} CreateProcessInfo;
		DBGKM_EXIT_THREAD ExitThread;
		DBGKM_EXIT_PROCESS ExitProcess;
		DBGKM_EXCEPTION Exception;
		DBGKM_LOAD_DLL LoadDll;
		DBGKM_UNLOAD_DLL UnloadDll;
	} StateInfo;
} DBGUI_WAIT_STATE_CHANGE, *PDBGUI_WAIT_STATE_CHANGE;

typedef struct _SYSTEM_PROCESS_INFORMATION {
	ULONG NextEntryOffset;
	BYTE Reserved1[52];
	PVOID Reserved2[3];
	HANDLE UniqueProcessId;
	HANDLE ParentProcessId;
	ULONG HandleCount;
	BYTE Reserved4[4];
	PVOID Reserved5[11];
	SIZE_T PeakPagefileUsage;
	SIZE_T PrivatePageCount;
	LARGE_INTEGER Reserved6[6];
} SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;

typedef LONG KPRIORITY;

typedef struct _THREAD_BASIC_INFORMATION {
	NTSTATUS ExitStatus;
	PVOID TebBaseAddress;
	CLIENT_ID ClientId;
	KAFFINITY AffinityMask;
	KPRIORITY Priority;
	KPRIORITY BasePriority;
} THREAD_BASIC_INFORMATION, *PTHREAD_BASIC_INFORMATION;

// hook function pointer declarations
typedef NTSTATUS (NTAPI *NtQueryInformationProcessFPtr)(HANDLE ProcessHandle, PROCESS_INFORMATION_CLASS ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength);
typedef NTSTATUS (NTAPI *NtSetInformationThreadFPtr)(HANDLE ThreadHandle, THREAD_INFORMATION_CLASS ThreadInformationClass, PVOID ThreadInformation, ULONG ThreadInformationLength);
typedef NTSTATUS (NTAPI *NtQueryObjectFPtr)(HANDLE ObjectHandle, OBJECT_INFORMATION_CLASS ObjectInformationClass, PVOID ObjectInformation, ULONG Length, PULONG ResultLength);
typedef NTSTATUS (NTAPI *NtCloseFPtr)(HANDLE ObjectHandle);
typedef NTSTATUS (NTAPI *NtQuerySystemInformationFPtr)(SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength);
typedef VOID (WINAPI *OutputDebugStringAFPtr)(LPCSTR lpOutputString);
typedef VOID (WINAPI *OutputDebugStringWFPtr)(LPCWSTR lpOutputString);
typedef DWORD (WINAPI *GetTickCountFPtr)();
typedef BOOL (WINAPI *BlockInputFPtr)(BOOL fBlockIt);
typedef NTSTATUS (NTAPI *DbgUiConvertStateChangeStructureFPtr)(PDBGUI_WAIT_STATE_CHANGE WaitStateChange, LPDEBUG_EVENT DebugEvent);
typedef HANDLE (WINAPI *OpenProcessFPtr)(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId);
typedef BOOL (WINAPI *SwitchDesktopFPtr)(HDESK hDesktop);
typedef VOID (NTAPI *KiUserExceptionDispatcherFPtr)(PEXCEPTION_RECORD pExcptRec, PCONTEXT pContext);
typedef NTSTATUS (NTAPI *NtSetContextThreadFPtr)(HANDLE hThread, const CONTEXT* lpContext);
typedef NTSTATUS (NTAPI *NtGetContextThreadFPtr)(HANDLE hThread, LPCONTEXT lpContext);
typedef NTSTATUS (NTAPI *NtYieldExecutionFPtr)();
typedef HWND (WINAPI *FindWindowAFPtr)(LPCSTR lpClassName, LPCSTR lpWindowName);
typedef HWND (WINAPI *FindWindowWFPtr)(LPCWSTR lpClassName, LPCWSTR lpWindowName);
typedef HWND (WINAPI *FindWindowExAFPtr)(HWND hWndParent, HWND hWndChildAfter, LPCSTR lpszClass, LPCSTR lpszWindow);
typedef HWND (WINAPI *FindWindowExWFPtr)(HWND hWndParent, HWND hWndChildAfter, LPCWSTR lpszClass, LPCWSTR lpszWindow);
typedef BOOL (WINAPI *EnumWindowsFPtr)(WNDENUMPROC lpEnumFunc, LPARAM lParam);
typedef DWORD (WINAPI *GetVersionFPtr)();
typedef NTSTATUS (NTAPI *NtTherminateProcessFPtr)(HANDLE ProcessHandle, NTSTATUS ExitStatus);
typedef NTSTATUS (NTAPI *NtTerminateThreadFPtr)(HANDLE ThreadHandle, NTSTATUS ExitStatus);
typedef NTSTATUS (NTAPI *RtlGetVersionFPtr)(PRTL_OSVERSIONINFOW lpVersionInformation);
typedef LONG (NTAPI *PVECTORED_EXCEPTION_HANDLER)(_EXCEPTION_POINTERS *ExceptionInfo);
typedef PVOID (WINAPI *AddVectoredExceptionHandlerFPtr)(ULONG First, PVECTORED_EXCEPTION_HANDLER Handler);
typedef ULONG (WINAPI *RemoveVectoredExceptionHandlerFPtr)(PVOID Handler);
typedef BOOL (WINAPI *DebugActiveProcessFPtr)(DWORD dwProcessId);