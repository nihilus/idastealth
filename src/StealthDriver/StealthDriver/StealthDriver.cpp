///////////////////////////////////////////////////////////////////////////////
///
/// Copyright (c) 2009 - <company name here>
///
/// Original filename: StealthDriver.cpp
/// Project          : StealthDriver
/// Date of creation : 2009-11-27
/// Author(s)        : Jan Newger
///
/// Purpose          : <description>
///
/// Revisions:
///  0000 [2009-11-27] Initial revision.
///
///////////////////////////////////////////////////////////////////////////////

// $Id$

#include <ntddk.h>
#include "StealthDriver.h"
#include "StealthImplementation.h"
#include <string.h>

namespace
{
	PDRIVER_OBJECT pdoGlobalDrvObj = 0;
	PRESET_UNICODE_STRING(usDeviceName, STEALTH_DEVICE_NAME);
	PRESET_UNICODE_STRING(usSymlinkName, STEALTH_SYMLINK_NAME);
}

NTSTATUS STEALTHDRIVER_DispatchCreateClose(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	NTSTATUS status = STATUS_SUCCESS;
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

NTSTATUS handleHook(PIRP Irp, PIO_STACK_LOCATION irpSp)
{
	__try
	{
		if (irpSp->Parameters.DeviceIoControl.InputBufferLength == sizeof(int))
		{
			StealthHook hookType = *(StealthHook*)Irp->AssociatedIrp.SystemBuffer;
			hookSysCall(hookType);
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return STATUS_INVALID_PARAMETER;
	}
	return STATUS_SUCCESS;
}

NTSTATUS STEALTHDRIVER_DispatchDeviceControl(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

	switch(irpSp->Parameters.DeviceIoControl.IoControlCode)
	{
	case IOCTL_STEALTHDRIVER_ENABLE_HOOKS:
		status = handleHook(Irp, irpSp);
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

VOID STEALTHDRIVER_DriverUnload(IN PDRIVER_OBJECT DriverObject)
{
	PDEVICE_OBJECT pdoNextDeviceObj = pdoGlobalDrvObj->DeviceObject;
	IoDeleteSymbolicLink(&usSymlinkName);

	// Delete all the device objects
	while(pdoNextDeviceObj)
	{
		PDEVICE_OBJECT pdoThisDeviceObj = pdoNextDeviceObj;
		pdoNextDeviceObj = pdoThisDeviceObj->NextDevice;
		IoDeleteDevice(pdoThisDeviceObj);
	}

	// remove hooks
	unhookSysCall(SH_NtSetInformationThread);
	unhookSysCall(SH_NtQueryInformationProcess);
}

#ifdef __cplusplus
extern "C" {
#endif
NTSTATUS DriverEntry(IN OUT PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
	PDEVICE_OBJECT pdoDeviceObj = 0;
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pdoGlobalDrvObj = DriverObject;

	// Create the device object.
	if(!NT_SUCCESS(status = IoCreateDevice(DriverObject, 0, &usDeviceName,
		FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &pdoDeviceObj)))
	{
		// Bail out (implicitly forces the driver to unload).
		return status;
	}

	// Now create the respective symbolic link object
	if(!NT_SUCCESS(status = IoCreateSymbolicLink(&usSymlinkName, &usDeviceName)))
	{
		IoDeleteDevice(pdoDeviceObj);
		return status;
	}

	if (!initSysCallHooking()) return STATUS_UNSUCCESSFUL;

	// NOTE: You need not provide your own implementation for any major function that
	//       you do not want to handle. I have seen code using DDKWizard that left the
	//       *empty* dispatch routines intact. This is not necessary at all!
	DriverObject->MajorFunction[IRP_MJ_CREATE] =
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = STEALTHDRIVER_DispatchCreateClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = STEALTHDRIVER_DispatchDeviceControl;
	DriverObject->DriverUnload = STEALTHDRIVER_DriverUnload;

	return STATUS_SUCCESS;
}
#ifdef __cplusplus
}; // extern "C"
#endif