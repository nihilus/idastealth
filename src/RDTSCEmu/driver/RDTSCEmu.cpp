///////////////////////////////////////////////////////////////////////////////
///
/// Copyright (c) 2009 - <company name here>
///
/// Original filename: RDTSCEmu.c
/// Project          : RDTSCEmu
/// Date of creation : 2009-01-01
/// Author(s)        : Jan Newger
///
/// Purpose          : Turns RDTS into a priv. instruction; fakes return values
///
/// Revisions:
///  0000 [2009-01-01] Initial revision.
///
///////////////////////////////////////////////////////////////////////////////

// $Id$

#ifdef __cplusplus
extern "C" {
#endif
#include <Fltkernel.h>
#include <ntddk.h>
#include <string.h>
#include <ntstrsafe.h>
#ifdef __cplusplus
}; // extern "C"
#endif

#include "RDTSCEmu.h"
#include "../distorm.h"
#include "HookInt.h"

#ifdef __cplusplus
namespace { // anonymous namespace to limit the scope of this global variable!
#endif
PDRIVER_OBJECT pdoGlobalDrvObj = 0;
#ifdef __cplusplus
}; // anonymous namespace
#endif

#define CR4_TO_EAX		__asm _emit 0x0F \
						__asm _emit 0x20 \
						__asm _emit 0xE0

#define EAX_TO_CR4		__asm _emit 0x0F \
						__asm _emit 0x22 \
						__asm _emit 0xE0

#define SET_TSD_EAX		__asm or	eax, 4
#define CLR_TSD_EAX		__asm and	eax, 0xFFFFFFFB

#define ENABLE_TSD		CR4_TO_EAX	\
						SET_TSD_EAX \
						EAX_TO_CR4

#define CLEAR_TSD		CR4_TO_EAX	\
						CLR_TSD_EAX \
						EAX_TO_CR4

#define MAX_CPUS		(32)
#define MAX_INSTR		(15)

// valid for all exceptions with an associated error code (see Intel manuals Vol.3A, 5.13)
typedef struct
{
	ULONG errorCode;
	ULONG eip;
	ULONG cs;
	ULONG eflags;
	ULONG esp;
	ULONG ss;
} STACK_WITHERR;

// integer register context and segment selectors
typedef struct
{
	ULONG gs;
	ULONG fs;
	ULONG es;
	ULONG ds;
	ULONG edi;
	ULONG esi;
	ULONG ebp;
	ULONG esp;
	ULONG ebx;
	ULONG edx;
	ULONG ecx;
	ULONG eax;	
} CTX_SEL;

// represents the stack layout at interrupt handler entry after all registers and segment
// selectors have been saved
typedef struct  
{
	CTX_SEL context;
	STACK_WITHERR origHandlerStack;
} STACK_WITHCTX, *PSTACK_WITHCTX;

UINT_PTR origHandlers[MAX_CPUS];
UNICODE_STRING deviceName = {0, 0, 0};
UNICODE_STRING dosDeviceName = {0, 0, 0};

// emulation parameters
bool methodIncreasing = false;
ULONG delta = 0;
ULONGLONG rdtscValue = 0;
ULONG constValue = 0;

// returns length of instruction if it has been identified as RDTSC
ULONG isRDTSC(PVOID address)
{
	__try
	{
		_DecodedInst instructions[MAX_INSTR];
		unsigned int instructionCount;
		_DecodeResult res = distorm_decode(0, (const unsigned char*)address, MAX_INSTR, Decode32Bits, instructions, MAX_INSTR, &instructionCount);
		if (res)
		{
			return strcmp((const char*)instructions->mnemonic.p, "RDTSC") ? 0 : instructions->size;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	}
	return 0;
}

// performs the actual emulation
// return false if original handler should be executed, true otherwise
bool __stdcall hookImplementation(PSTACK_WITHCTX stackLayout)
{
	if (MmIsAddressValid((PVOID)stackLayout->origHandlerStack.eip))
	{
		if (ULONG length = isRDTSC((PVOID)stackLayout->origHandlerStack.eip))
		{
			if (methodIncreasing)
			{
				static ULONG seed = 0x666;
				if (delta) rdtscValue += RtlRandomEx(&seed) % delta;
				stackLayout->context.eax = (ULONG)rdtscValue;
				stackLayout->context.edx = (ULONG)(rdtscValue >> 32);
			}
			else
			{
				stackLayout->context.eax = stackLayout->context.edx = constValue;
			}
			// #GP is a fault, so the CPU would restart the faulting instruction
			// since we "handled" this exception, we need to skip it
			stackLayout->origHandlerStack.eip += length;
			return true;
		}
	}
	
	return false;
}

// stack layout at handler entry is reflected via the
// STACK_WITHERR structure (see Intel manuals Vol.3A, 5.12.1)
// after registers and segment selectors have been saved, the stack layout
// is equivalent to the STACK_WITHCTX structure
VOID __declspec(naked) hookStub()
{
	__asm
	{
		pushad
		push	ds
		push    es
		push    fs
		push    gs
		// set kernel mode selectors
		mov     ax, 0x23
		mov     ds, ax
		mov     es, ax
		mov     gs, ax
		mov     ax, 0x30
		mov     fs, ax

		push	esp
		call	hookImplementation
		cmp		al, 0
		jz		oldHandler

		pop		gs
		pop		fs
		pop		es
		pop		ds
		popad	
		// we need to remove the error code manually (see Intel manuals Vol.3A, 5.13)
		add		esp, 4
		iretd
		
		// just call first original handler
		oldHandler:
		pop		gs
		pop		fs
		pop		es
		pop		ds
		popad
		jmp		dword ptr [origHandlers]
	}
}

void initializeHooks()
{
	// load CR4 register into EAX, set TSD flag and update CR4 from EAX
	for (CCHAR i=0; i<KeNumberProcessors; ++i)
	{
		switchToCPU(i);
		hookInterrupt(hookStub, 0xD, &origHandlers[i]);
		ENABLE_TSD;
	}
}

void removeHooks()
{
	for (CCHAR i=0; i<KeNumberProcessors; ++i)
	{
		switchToCPU(i);
		CLEAR_TSD;
		hookInterrupt((PVOID)origHandlers[i], 0xD, NULL);
	}
}

void freeStrings()
{
	if (deviceName.Buffer) ExFreePool(deviceName.Buffer);
	if (dosDeviceName.Buffer) ExFreePool(dosDeviceName.Buffer);
	deviceName.Buffer = NULL;
	dosDeviceName.Buffer = NULL;
}

NTSTATUS RDTSCEMU_DispatchCreateClose(IN PDEVICE_OBJECT	DeviceObject, IN PIRP Irp)
{
	NTSTATUS status = STATUS_SUCCESS;
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

NTSTATUS RDTSCEMU_DispatchDeviceControl(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

	switch(irpSp->Parameters.DeviceIoControl.IoControlCode)
	{
	case IOCTL_RDTSCEMU_METHOD_ALWAYS_CONST:
		__try
		{
			if (irpSp->Parameters.DeviceIoControl.InputBufferLength == sizeof(ULONG))
			{
				methodIncreasing = false;
				constValue = *(ULONG*)Irp->AssociatedIrp.SystemBuffer;
			}
			else status = STATUS_INVALID_PARAMETER;
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			status = STATUS_INVALID_PARAMETER;
		}
		break;

	case IOCTL_RDTSCEMU_METHOD_INCREASING:
		__try
		{
			if (irpSp->Parameters.DeviceIoControl.InputBufferLength == sizeof(ULONG))
			{
				__asm
				{
					push	eax
					push	ecx
					push	edx
					rdtsc
					lea		ecx, rdtscValue
					mov		dword ptr [ecx], eax
					mov		dword ptr [ecx+4], edx
					pop		edx
					pop		ecx
					pop		eax
				}
				// set delta
				delta = *(PULONG)(Irp->AssociatedIrp.SystemBuffer);
				methodIncreasing = true;
				Irp->IoStatus.Status = STATUS_SUCCESS;
			}
			else Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			status = STATUS_INVALID_PARAMETER;
		}
		break;

	default:
		Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
		Irp->IoStatus.Information = 0;
		break;
	}

	status = Irp->IoStatus.Status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

VOID RDTSCEMU_DriverUnload(IN PDRIVER_OBJECT DriverObject)
{
	PDEVICE_OBJECT pdoNextDeviceObj = pdoGlobalDrvObj->DeviceObject;
	IoDeleteSymbolicLink(&dosDeviceName);

	// Delete all the device objects
	while(pdoNextDeviceObj)
	{
		PDEVICE_OBJECT pdoThisDeviceObj = pdoNextDeviceObj;
		pdoNextDeviceObj = pdoThisDeviceObj->NextDevice;
		IoDeleteDevice(pdoThisDeviceObj);
	}
	removeHooks();
	freeStrings();
}

// we get our own name so we can create the device objects based on this
// this allows us to be started with a random name
// (rdtscemu and similar strings are recognized by some packers)
PWSTR getRandomizedName(PUNICODE_STRING registryPath)
{
	UNICODE_STRING registryKey;
	UNICODE_STRING valueName;
	OBJECT_ATTRIBUTES objAttr;
	HANDLE hKey;
	
	RtlInitUnicodeString(&valueName, L"DisplayName");
	InitializeObjectAttributes(&objAttr, registryPath, OBJ_KERNEL_HANDLE, NULL, NULL);

	PWSTR result = NULL;

	NTSTATUS status = ZwOpenKey(&hKey, KEY_READ, &objAttr);
	if (NT_SUCCESS(status))
	{
		ULONG size = 0;
		status = ZwQueryValueKey(hKey, &valueName, KeyValuePartialInformation, NULL, 0, &size);
		if (status == STATUS_BUFFER_TOO_SMALL && size > 0)
		{ 
			PKEY_VALUE_PARTIAL_INFORMATION vpip = (PKEY_VALUE_PARTIAL_INFORMATION)ExAllocatePool(PagedPool, size);
			if (vpip)
			{
				status = ZwQueryValueKey(hKey, &valueName, KeyValuePartialInformation, vpip, size, &size);
				if (NT_SUCCESS(status))
				{
					result = (PWSTR)ExAllocatePool(PagedPool, vpip->DataLength);
					RtlStringCbCopyW(result, vpip->DataLength, (PCWSTR)vpip->Data);
					ExFreePool(vpip);
				}
			}
		}
		ZwClose(hKey);
	}

	return result;
}

bool buildDeviceName(PUNICODE_STRING dest, PCWSTR devName, PCWSTR randomPart)
{
	size_t bufLen = (wcslen(devName) + wcslen(randomPart) + 1) * sizeof(WCHAR);
	PWSTR devNameBuf = (PWSTR)ExAllocatePool(PagedPool, bufLen);
	if (!devNameBuf) return false;

	RtlStringCbCopyW(devNameBuf, bufLen, devName);
	RtlStringCbCatW(devNameBuf, bufLen, randomPart);
	RtlInitUnicodeString(dest, devNameBuf);
	return true;
}

// build device name by examining 
bool initDeviceStrings(PUNICODE_STRING registryPath)
{
	PWSTR randName = getRandomizedName(registryPath);
	if (!randName) return false;
	bool retVal = false;
	if (buildDeviceName(&deviceName, WIDESTRING(DEVICE_NAME), randName)
		&& buildDeviceName(&dosDeviceName, WIDESTRING(SYMLINK_NAME), randName))
	{
		retVal = true;
	}
	ExFreePool(randName);
	return retVal;
}

#ifdef __cplusplus
extern "C" {
#endif
NTSTATUS DriverEntry(IN OUT PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
	PDEVICE_OBJECT pdoDeviceObj = 0;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pdoGlobalDrvObj = DriverObject;

	if (!initDeviceStrings(RegistryPath)) return status;	

	// Create the device object.
	if(!NT_SUCCESS(status = IoCreateDevice(
		DriverObject,
		0,
		&deviceName,
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN,
		FALSE,
		&pdoDeviceObj
		)))
	{
		// Bail out (implicitly forces the driver to unload).
		return status;
	};

	// Now create the respective symbolic link object
	if(!NT_SUCCESS(status = IoCreateSymbolicLink(
		&dosDeviceName,
		&deviceName
		)))
	{
		IoDeleteDevice(pdoDeviceObj);
		return status;
	}

	// NOTE: You need not provide your own implementation for any major function that
	//       you do not want to handle. I have seen code using DDKWizard that left the
	//       *empty* dispatch routines intact. This is not necessary at all!
	DriverObject->MajorFunction[IRP_MJ_CREATE] =
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = RDTSCEMU_DispatchCreateClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = RDTSCEMU_DispatchDeviceControl;
	DriverObject->DriverUnload = RDTSCEMU_DriverUnload;	

	initializeHooks();

	return STATUS_SUCCESS;
}
#ifdef __cplusplus
}; // extern "C"
#endif
