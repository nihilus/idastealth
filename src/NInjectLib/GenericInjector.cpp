#include "GenericInjector.h"

// TODO: exceptions hinzufügen!
GenericInjector::GenericInjector(const Process& process)
	: process_(process),
	injectedData_(NULL),
	injectedCode_(NULL)
{
}

GenericInjector::~GenericInjector()
{
	freeMem();	
}

// free memory and code in the remote process
void GenericInjector::freeMem()
{
	if (injectedData_ != NULL) process_.freeMem(injectedData_);
	if (injectedCode_ != NULL) process_.freeMem(injectedCode_);
	injectedData_ = injectedCode_ = NULL;
}

// inject code + data into remote process and pass data as param to the injected code
void GenericInjector::doInjection(INJECT_DATAPAYLOAD dataPayload, INJECT_CODEPAYLOAD codePayload)
{
	freeMem();
	// allocate memory in target process and write data struct
	injectedData_ = process_.allocMem(dataPayload.size);
	process_.writeMemory(injectedData_, dataPayload.data, dataPayload.size);

	// inject code payload
	injectedCode_ = process_.allocMem(codePayload.size);
	process_.writeMemory(injectedCode_, codePayload.code, codePayload.size);
	process_.startThread(injectedCode_, injectedData_);
	// wait for thread to finish
	process_.waitForThread();
}

void* GenericInjector::getAddrOfData() const
{
	return injectedData_;
}

void* GenericInjector::getAddrOfCode() const
{
	return injectedCode_;
}