#include "RemoteLibCall.h"
#include <tchar.h>

using namespace std;

#pragma pack(push, 1)

// this struct holds all data needed to perform a remote call
struct REMOTELIB_DATA
{
	void* getProcAddress;
	HMODULE hDll;
	int functionNr;
	int errorFlag;
	char functionName[MaxFuncNameLength];
};

#pragma pack(pop)

RemoteLibCall::RemoteLibCall(const InjectLibrary& library, const Process& process)
	: process_(process)
{
	hDll_ = library.getDllHandle();
	if (hDll_ == NULL) throw exception("Invalid dll handle");
}

RemoteLibCall::~RemoteLibCall()
{
}

// preform remote call by name
bool RemoteLibCall::remoteCall(const string& functionName)
{
	if (functionName.length() >= MaxFuncNameLength) throw exception("Function name too long!");
	functionName_ = functionName;
	functionNumber_ = -1;
	return remoteCall();
}

// perform remote call by ordinal
bool RemoteLibCall::remoteCall(unsigned int exportNumber)
{
	functionNumber_ = exportNumber;
	functionName_ = "";
	return remoteCall();
}

// create specific data/code payloads and use genericinjector to start the remote code
// returns true if remote call succeeded otherwise false
bool RemoteLibCall::remoteCall()
{
	GenericInjector injector(process_);
	INJECT_DATAPAYLOAD data = createDataPayload();
	INJECT_CODEPAYLOAD code = createCodePayload();
	bool noError = false;
	injector.doInjection(data, code);
	unsigned int* errorFlagAddr = (unsigned int*)injector.getAddrOfData();
	errorFlagAddr += 12;
	unsigned int errorFlag;
	process_.readMemory(errorFlagAddr, &errorFlag, sizeof(unsigned int));
	noError = (errorFlag != 0);

	free(data.data);
	free(code.code);
	return noError;
}

INJECT_DATAPAYLOAD RemoteLibCall::createDataPayload()
{
	REMOTELIB_DATA* tmpData = (REMOTELIB_DATA*)malloc(sizeof(REMOTELIB_DATA));
	// set functionNr to -1 if we call by name
	if (functionNumber_ == -1) 
	{
		strcpy_s(tmpData->functionName, MaxFuncNameLength, functionName_.c_str());
		tmpData->functionNr = -1;
	}
	else tmpData->functionNr = functionNumber_;

	// the remote process needs the actual address of GetProcAddress and our dll handle
	HMODULE hKernel32 = LoadLibrary(_T("kernel32.dll"));
	tmpData->getProcAddress = GetProcAddress(hKernel32, "GetProcAddress");
	// save handle of the dll we want to call
	tmpData->hDll = hDll_;

	INJECT_DATAPAYLOAD injectData;
	injectData.data = tmpData;
	injectData.size = sizeof(REMOTELIB_DATA);
	return injectData;
}

#pragma warning(disable : 4731 4740) 

//create code to perform remote call
INJECT_CODEPAYLOAD RemoteLibCall::createCodePayload()
{
	size_t s, e;
	void* source;

	__asm
	{
		mov s, offset start
		mov e, offset end
		mov source, offset start
		jmp end // we only want to copy this code - not execute it so jump over it
	start:
		// create standard stack frame
		push ebp
		mov ebp, esp
		push ebx
		push esi
		push edi
		
		mov esi, [ebp+8] // get first param, ESI now points to strcut
		mov edi, [esi] // get getprocaddress pointer
		mov ebx, [esi+8] // get functionNr
		mov ecx, [esi+4] // get module handle
		cmp ebx, 0xFFFFFFFF
		jnz byOrdinal
		lea ebx, [esi+16] // get pointer to function name
		
	byOrdinal:
		push ebx
		push ecx
		call edi // call getprocaddress
		test eax, eax // could we resolve the funtion?
		jz error
		call eax // call exported function
		mov eax, 1
		jmp finished

	error:
		xor eax, eax

	finished:
		mov [esi+12], eax // save error flag
		pop edi
		pop esi
		pop ebx
		pop ebp
		//pop ebx
		//pop esi
		//pop edi
		//pop ebp
		ret
	end:
	}

	INJECT_CODEPAYLOAD tmpPayload;
	tmpPayload.size = e - s;
	tmpPayload.code = malloc(tmpPayload.size);
	// copying code from code section should always work
	memcpy(tmpPayload.code, source, tmpPayload.size);
	return tmpPayload;
}

#pragma warning(default: 4731 4740)