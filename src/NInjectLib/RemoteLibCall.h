#pragma once

#include <iostream>
#include <Windows.h>

#include "InjectLib.h"
#include "GenericInjector.h"
#include "Process.h"

const int MaxFuncNameLength = 42;

// class to perform a procedure call to an dll which was previously injected into a remote process
class RemoteLibCall
{
public:
	RemoteLibCall(const InjectLibrary& library, const Process& process);
	~RemoteLibCall();
	bool remoteCall(const std::string& functionName);
	bool remoteCall(unsigned int exportNumber);

private:
	bool remoteCall();
	INJECT_DATAPAYLOAD createDataPayload();
	INJECT_CODEPAYLOAD createCodePayload();

	Process process_;
	HMODULE hDll_;
	std::string functionName_;
	int functionNumber_;
};
