///////////////////////////////////////////////////////////////////////////////
///
/// Copyright (c) 2009 - <company name here>
///
/// Original filename: RDTSCEmu.h
/// Project          : RDTSCEmu
/// Date of creation : <see RDTSCEmu.c>
/// Author(s)        : <see RDTSCEmu.c>
///
/// Purpose          : <see RDTSCEmu.c>
///
/// Revisions:         <see RDTSCEmu.c>
///
///////////////////////////////////////////////////////////////////////////////

// $Id$

#ifndef __RDTSCEMU_H_VERSION__
#define __RDTSCEMU_H_VERSION__ 100

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "drvcommon.h"
#include "drvversion.h"

#define DEVICE_NAME			"\\Device\\"
#define SYMLINK_NAME		"\\DosDevices\\"

#ifndef FILE_DEVICE_RDTSCEMU
#define FILE_DEVICE_RDTSCEMU 0x8000
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

enum RDTSCMode
{
	constant,
	increasing
};

#define IOCTL_RDTSCEMU_METHOD_ALWAYS_CONST CTL_CODE(FILE_DEVICE_RDTSCEMU, 0x02, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
#define IOCTL_RDTSCEMU_METHOD_INCREASING CTL_CODE(FILE_DEVICE_RDTSCEMU, 0x03, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)

#endif // __RDTSCEMU_H_VERSION__
