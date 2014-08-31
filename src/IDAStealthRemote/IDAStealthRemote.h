#pragma once

// minimum OS is Windows 2000
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#include <boost/asio.hpp>

#pragma warning(disable : 4702)
#include <boost/lexical_cast.hpp>
#pragma warning(default : 4702)

#include <iostream>
#include "RemoteStealthServer.h"
#include <tchar.h>