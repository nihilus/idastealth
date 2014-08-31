#include "StealthImplementation.h"

// forward declarations

NTSTATUS NtSetInformationThreadHook(__in HANDLE ThreadHandle,
									__in THREADINFOCLASS ThreadInformationClass,
									__in_bcount(ThreadInformationLength) PVOID ThreadInformation,
									__in ULONG ThreadInformationLength);

NTSTATUS NtQueryInformationProcessHook(__in HANDLE ProcessHandle, 
									   __in PROCESSINFOCLASS ProcessInformationClass,
									   __out PVOID ProcessInformation,
									   __in ULONG ProcessInformationLength,
									   __out_opt  PULONG ReturnLength);


// each service table has an associated SSDT_Entry
#pragma pack(1)
struct SSDT_Entry
{
	uintptr_t* ServiceTableBase;
	uintptr_t* ServiceCounterTableBase;
	unsigned int NumberOfServices;
	unsigned char* ParamTableBase;
};
#pragma pack()

extern "C"
NTSYSAPI
NTSTATUS
NTAPI ZwQueryInformationProcess(__in HANDLE ProcessHandle,
								__in PROCESSINFOCLASS ProcessInformationClass,
								__out_bcount_opt(ProcessInformationLength) PVOID ProcessInformation,
								__in ULONG ProcessInformationLength,
								__out_opt PULONG ReturnLength);


extern "C" SSDT_Entry* KeServiceDescriptorTable;

typedef NTSTATUS (NTAPI *NtSetInformationThreadFPtr)(__in HANDLE ThreadHandle,
													 __in THREADINFOCLASS ThreadInformationClass,
													 __in_bcount(ThreadInformationLength) PVOID ThreadInformation,
													 __in ULONG ThreadInformationLength);
typedef NTSTATUS (NTAPI *NtQueryInformationProcessFPtr)(__in HANDLE ProcessHandle,
														__in PROCESSINFOCLASS ProcessInformationClass,
														__out_bcount_opt(ProcessInformationLength) PVOID ProcessInformation,
														__in ULONG ProcessInformationLength,
														__out_opt PULONG ReturnLength);

namespace
{
	PMDL mdlSysCallTable = NULL;
	uintptr_t* mappedSysCallTable = NULL;

	NtSetInformationThreadFPtr origNtSetInformationThread = NULL;
	NtQueryInformationProcessFPtr origNtQueryInformationProcess = NULL;
}

// hook entry in service descriptor table; returns pointer to original function
void hookEntry(void* zwFunction, void* hook, void** originalFunction)
{
	// first translate Zw* to Nt* function - the first instruction moves
	// in each Zw* routine moves the corresponding Nt* syscall index to a register
	// TODO: use disassembler to get Nt* index
	unsigned int index = *(unsigned int*)((unsigned char*)zwFunction + 1);
	void* oldEntry = (void*)InterlockedExchange((volatile LONG*)&mappedSysCallTable[index], (LONG)hook);
	if (originalFunction) *originalFunction = oldEntry;
}

void hookSysCall(StealthHook hook)
{
	switch (hook)
	{
	case SH_NtSetInformationThread:
		if (!origNtSetInformationThread)
			hookEntry(ZwSetInformationThread, NtSetInformationThreadHook, (void**)&origNtSetInformationThread);
		break;

	case SH_NtQueryInformationProcess:
		if (!origNtQueryInformationProcess)
			hookEntry(ZwQueryInformationProcess, NtQueryInformationProcessHook, (void**)&origNtQueryInformationProcess);
		break;
	}
}

void unhookSysCall(StealthHook hook)
{
	switch (hook)
	{
	case SH_NtSetInformationThread:
		if (origNtSetInformationThread)
		{
			hookEntry(ZwSetInformationThread, origNtSetInformationThread, NULL);
			origNtSetInformationThread = NULL;
		}
		break;

	case SH_NtQueryInformationProcess:
		if (origNtQueryInformationProcess)
		{
			hookEntry(ZwQueryInformationProcess, origNtQueryInformationProcess, NULL);
			origNtQueryInformationProcess = NULL;
		}
		break;
	}
}

// enable write access to service descriptor table
bool initSysCallHooking()
{
	if (mappedSysCallTable) return true;

	mdlSysCallTable = MmCreateMdl(NULL, KeServiceDescriptorTable->ServiceTableBase,
		KeServiceDescriptorTable->NumberOfServices*sizeof(uintptr_t));
	if (!mdlSysCallTable) return false;

	MmBuildMdlForNonPagedPool(mdlSysCallTable);
	mdlSysCallTable->MdlFlags = mdlSysCallTable->MdlFlags | MDL_MAPPED_TO_SYSTEM_VA;

	mappedSysCallTable = (uintptr_t*)MmMapLockedPages(mdlSysCallTable, KernelMode);
	return mappedSysCallTable != NULL;
}

// hook implementations

NTSTATUS NtSetInformationThreadHook(__in HANDLE ThreadHandle,
									__in THREADINFOCLASS ThreadInformationClass,
									__in_bcount(ThreadInformationLength) PVOID ThreadInformation,
									__in ULONG ThreadInformationLength)
{
	if (ThreadInformationClass == ThreadHideFromDebugger)
	{
		PKTHREAD obj;
		NTSTATUS retVal;
		if (NT_SUCCESS(retVal = ObReferenceObjectByHandle(ThreadHandle, 0, NULL,
														  KernelMode, (PVOID*)&obj, NULL)))
		{
			// handle is ok, so decrement reference count
			ObDereferenceObject(obj);
			return STATUS_SUCCESS;
		}
		else return retVal;
	}
	else return origNtSetInformationThread(ThreadHandle, ThreadInformationClass,
										   ThreadInformation, ThreadInformationLength);
}

NTSTATUS NtQueryInformationProcessHook(__in HANDLE ProcessHandle,
									   __in PROCESSINFOCLASS ProcessInformationClass,
									   __out_bcount_opt(ProcessInformationLength) PVOID ProcessInformation,
									   __in ULONG ProcessInformationLength,
									   __out_opt PULONG ReturnLength)
{
	NTSTATUS retVal = origNtQueryInformationProcess(ProcessHandle, ProcessInformationClass, ProcessInformation,
		ProcessInformationLength, ReturnLength);

	if (NT_SUCCESS(retVal))
	{
		switch (ProcessInformationClass)
		{
		case ProcessDebugObjectHandle:
			return STATUS_PORT_NOT_SET;

		case ProcessDebugPort:
			*(PULONG)ProcessInformation = 0;
			break;

		case ProcessDebugFlags:
			*(PULONG)ProcessInformation = 1;
			break;
		}
	}
	return retVal;
}