#include "InjectLib.h"

using namespace std;

#pragma pack(push, 1)

struct LOADLIB_DATA
{
	void* loadLibrary;
	char fileName[MAX_PATH];
	// the dll handle will be filled by the code in the remote process so we can read it back
	HMODULE hDll;
};

struct UNLOADLIB_DATA
{
	void* freeLibrary;
	HMODULE hDll;
	int errorFlag;
};

#pragma pack(pop)

InjectLibrary::InjectLibrary(const string& fileName, const Process& process) 
	: fileName_(fileName),
	process_(process)
{
	if (fileName.length() >= MAX_PATH) throw exception("Invalid library - filename too long!");
}

InjectLibrary::InjectLibrary(HMODULE hRemoteLib, const Process& process)
	: process_(process),
	hDll_(hRemoteLib)
{
}

InjectLibrary::~InjectLibrary()
{
}

INJECT_DATAPAYLOAD InjectLibrary::createLoadLibData()
{
	LOADLIB_DATA* tmpData = (LOADLIB_DATA*)malloc(sizeof(LOADLIB_DATA));
	strcpy_s(tmpData->fileName, MAX_PATH, fileName_.c_str());
	// we always use ANSI version of APIs
#ifdef UNICODE
	tmpData->loadLibrary = GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");
#else
	tmpData->loadLibrary = GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
#endif

	
	INJECT_DATAPAYLOAD dataPayload;
	dataPayload.data = tmpData;
	dataPayload.size = sizeof(LOADLIB_DATA);
	
	return dataPayload;
}

#pragma warning(disable : 4731 4740) 

INJECT_CODEPAYLOAD InjectLibrary::createLoadLibCode()
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
		push esi
		push edi

		mov esi, [ebp+8] // get first param, ESI now points to strcut
		mov edi, [esi] // get loadlibrary pointer
		lea ecx, [esi+4] // get addres of dll filename
		push ecx
		call edi // call loadlibrary
		lea ecx, [esi+MAX_PATH+4] // let EBX point to the handle entry in our struct
		mov [ecx], eax // save handle

		xor eax, eax
		pop edi
		pop esi
		pop ebp
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

#pragma warning(default : 4731 4740)

// inject code into remote process to load the library
bool InjectLibrary::injectLib()
{
	GenericInjector injector(process_);
	INJECT_DATAPAYLOAD data = createLoadLibData();
	INJECT_CODEPAYLOAD code = createLoadLibCode();
	injector.doInjection(data, code);
	free(data.data);
	free(code.code);

	// read back dll handle
	char* readAddress = (char*)injector.getAddrOfData();
	readAddress += MAX_PATH + sizeof(HANDLE);
	HMODULE hRemoteDll;
	process_.readMemory(readAddress, (void*)&hRemoteDll, sizeof(HMODULE));
	hDll_ = hRemoteDll;

	return (hDll_ == 0 ? false : true);
}

// unload lib from remote process
bool InjectLibrary::unloadLib()
{
	GenericInjector injector(process_);
	INJECT_CODEPAYLOAD code = createUnloadLibCode();
	INJECT_DATAPAYLOAD data = createUnloadLibData();
	bool errorFlag = false;
	injector.doInjection(data, code);

	// read back return value from freelibrary call
	char* readAddress = (char*)injector.getAddrOfData();
	readAddress += sizeof(void*) + sizeof(HMODULE);
	int error;
	process_.readMemory(readAddress, &error, sizeof(int));
	errorFlag = (error != 0);
	free(code.code);
	free(data.data);

	return errorFlag;
}

// create data struct with necessary information to unload lib from remote process
INJECT_DATAPAYLOAD InjectLibrary::createUnloadLibData()
{
	UNLOADLIB_DATA* tmpData = (UNLOADLIB_DATA*)malloc(sizeof(UNLOADLIB_DATA));
#ifdef UNICODE
	tmpData->freeLibrary = GetProcAddress(GetModuleHandle(L"kernel32.dll"), "FreeLibrary");
#else
	tmpData->freeLibrary = GetProcAddress(GetModuleHandle("kernel32.dll"), "FreeLibrary");
#endif
	tmpData->hDll = hDll_;
	
	INJECT_DATAPAYLOAD dataPayload;
	dataPayload.data = tmpData;
	dataPayload.size = sizeof(UNLOADLIB_DATA);

	return dataPayload;
}

#pragma warning(disable : 4731 4740) 

//create code to unload the lib from the remote process
INJECT_CODEPAYLOAD InjectLibrary::createUnloadLibCode()
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
		push esi

		mov esi, [ebp+8] // get first param, ESI now points to strcut
		mov eax, [esi] // get freelibrary pointer
		mov ecx, [esi+4] // get module handle
		push ecx
		call eax // call freelibrary
		lea ecx, [esi+8] // let EBX point to the errorflag field
		mov [ecx], eax

		xor eax, eax
		pop esi
		pop ebp
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

const Process& InjectLibrary::getProcess() const
{
	return process_;
}

HMODULE InjectLibrary::getDllHandle() const
{
	return hDll_;
}