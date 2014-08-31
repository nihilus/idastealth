#pragma once

#include <ntddk.h>
#include "StealthDriver.h"

void hookSysCall(StealthHook hook);
void unhookSysCall(StealthHook hook);
bool initSysCallHooking();