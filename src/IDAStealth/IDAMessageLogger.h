#pragma once

// provides functionality to print messages to IDAs output window
// can be called from any thread safely

#include <iostream>
#include <Windows.h>

class IDAMessageLogger
{
public:
	IDAMessageLogger();
	~IDAMessageLogger() {}

	void log(const std::string& str);

private:

	HWND hIDAWnd_;
	unsigned int idaMainThread_;
};