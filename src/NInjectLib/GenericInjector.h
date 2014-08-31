#pragma once

#include <iostream>
#include "Process.h"

struct INJECT_DATAPAYLOAD 
{
	void* data;
	size_t size;
};

struct INJECT_CODEPAYLOAD 
{
	void* code;
	size_t size;
};

// abstract base class for injection of code and data into a remote process
// derived classes need to implement methods that generate specific code and data payload
class GenericInjector
{
public:
	GenericInjector(const Process& process);
	~GenericInjector();

	void doInjection(INJECT_DATAPAYLOAD dataPayload, INJECT_CODEPAYLOAD codePayload);
	void* getAddrOfData() const;
	void* getAddrOfCode() const;

private:
	void freeMem();
	Process process_;
	void* injectedData_;
	void* injectedCode_;
};
