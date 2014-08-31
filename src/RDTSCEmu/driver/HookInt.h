#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <ntddk.h>
#include <ntdef.h>
#ifdef __cplusplus
}; // extern "C"
#endif

VOID hookInterrupt(PVOID newHandler, ULONG number, PUINT_PTR oldHandler);
VOID switchToCPU(CCHAR cpu);