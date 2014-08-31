#pragma once

#include <Windows.h>
#include <iostream>

#include "GenericInjector.h"
#include "Process.h"

// class to inject a dll into a remote process which is already running
class InjectLibrary
{
public:
	InjectLibrary(const std::string& fileName, const Process& process);
	InjectLibrary(HMODULE hRemoteLib, const Process& process);
	~InjectLibrary();
	HMODULE getDllHandle() const;
	const Process& getProcess() const;
	bool injectLib();
	bool unloadLib();

private:
	INJECT_DATAPAYLOAD createLoadLibData();
	INJECT_CODEPAYLOAD createLoadLibCode();
	INJECT_DATAPAYLOAD createUnloadLibData();
	INJECT_CODEPAYLOAD createUnloadLibCode();

	std::string fileName_;
	Process process_;
	HMODULE hDll_;
};
