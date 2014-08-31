///////////////////////////////////////////////////////////////////////////////
///
/// Copyright (c) 2009 - <company name here>
///
/// Original filename: StealthDriver.h
/// Project          : StealthDriver
/// Date of creation : <see StealthDriver.cpp>
/// Author(s)        : <see StealthDriver.cpp>
///
/// Purpose          : <see StealthDriver.cpp>
///
/// Revisions:         <see StealthDriver.cpp>
///
///////////////////////////////////////////////////////////////////////////////

// $Id$

#ifndef __STEALTHDRIVER_H_VERSION__
#define __STEALTHDRIVER_H_VERSION__ 100

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "drvcommon.h"
#include "drvversion.h"

#define STEALTH_DEVICE_NAME			"\\Device\\STEALTHDRIVER"
#define STEALTH_SYMLINK_NAME		"\\DosDevices\\STEALTHDRIVER"

#ifndef FILE_DEVICE_STEALTHDRIVER
#define FILE_DEVICE_STEALTHDRIVER 0x8000
#endif

// Values defined for "Method"
// METHOD_BUFFERED
// METHOD_IN_DIRECT
// METHOD_OUT_DIRECT
// METHOD_NEITHER
// 
// Values defined for "Access"
// FILE_ANY_ACCESS
// FILE_READ_ACCESS
// FILE_WRITE_ACCESS

// hook flags
enum StealthHook
{
	SH_NtSetInformationThread,
	SH_NtQueryInformationProcess
};

const ULONG IOCTL_STEALTHDRIVER_ENABLE_HOOKS = (ULONG)CTL_CODE(FILE_DEVICE_STEALTHDRIVER, 0x801, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA);

#endif // __STEALTHDRIVER_H_VERSION__
