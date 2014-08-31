#include "HookInt.h"

extern "C" KAFFINITY KeSetAffinityThread(IN PKTHREAD Thread, IN KAFFINITY Affinity);

#define RESTORE_INTS	
#define DISABLE_INTS	

#define MAKELONG(a, b) ((ULONG)(((USHORT)(((ULONG)(a)) & 0xffff)) | ((ULONG)((USHORT)(((ULONG)(b)) & 0xffff))) << 16))

// layout of data struct retrieved from sidt instruction
typedef struct
{
	USHORT limit;
	USHORT lowBase;
	USHORT highBase;
} IDT_INFO, *PIDT_INFO;

// simplified layout of an IDT entry 
// (all the flag details have been omitted, we don't change them anyway)
typedef struct  
{
	USHORT lowOffset;
	USHORT segSelector;
	USHORT flags;
	USHORT highOffset;
} IDT_ENTRY, *PIDT_ENTRY;

IDT_INFO getIDTInfo()
{
	IDT_INFO retVal;
	__asm
	{
		sidt	retVal
		//mov eax, fs:[0]KPCR.IDT
		//mov retVal, eax
	}
	return retVal;
}

VOID hookInterrupt(PVOID newHandler, ULONG number, PUINT_PTR oldHandler)
{
	IDT_INFO info;
	__asm	sidt info;
	PIDT_ENTRY idt = (PIDT_ENTRY)MAKELONG(info.lowBase, info.highBase);

	// save EFLAGS, then disable interrupts
	__asm	pushfd
	__asm	cli

	UINT_PTR origHandler = (ULONG)(idt[number].highOffset) << 16 | idt[number].lowOffset;

	idt[number].lowOffset = (USHORT)newHandler;
	idt[number].highOffset = (USHORT)((ULONG)newHandler >> 16);
	if (oldHandler) *oldHandler = origHandler;

	// CLI just clears the IF in EFLAGS so we don't need to execute STI here
	// by popping the previously pushed EFLAGS we revert to the original state
	__asm	popfd
}

VOID switchToCPU(CCHAR cpu)
{
	KeSetAffinityThread(KeGetCurrentThread(), 1 << cpu);
}