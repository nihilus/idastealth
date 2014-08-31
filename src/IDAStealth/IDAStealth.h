#pragma once

#include "IDACommon.h"
#include <iostream>

int idaapi callback(void* user_data, int notification_code, va_list va);
void localStealth();