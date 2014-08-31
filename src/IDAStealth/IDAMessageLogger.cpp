#include "IDAMessageLogger.h"
#include "IDACommon.h"

namespace
{
	const int TimerIntervall = 66;
	std::string message_;
}

IDAMessageLogger::IDAMessageLogger()
{
	hIDAWnd_ = (HWND)callui(ui_get_hwnd).vptr;
	idaMainThread_ = GetWindowThreadProcessId(hIDAWnd_, NULL);
}

static void CALLBACK timerCallbackProc(HWND hwnd, UINT, UINT idTimer, DWORD)
{
	if (message_.size())
	{
		msg("%s", message_.c_str());
		message_ = "";
	}
	KillTimer(hwnd, idTimer);
}

void IDAMessageLogger::log(const std::string& str)
{
	if (GetCurrentThreadId() == idaMainThread_) msg(str.c_str());
	else
	{
		message_ = str;
		UINT_PTR id = SetTimer(0, 0, TimerIntervall, NULL);
		SetTimer(hIDAWnd_, id, TimerIntervall, (TIMERPROC)timerCallbackProc);
	}
}