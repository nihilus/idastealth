#include "RDTSCDriver.h"
#include <Windows.h>
#include <sstream>
#include <boost/filesystem.hpp>

using namespace std;
using namespace boost::filesystem;

RDTSCDriver::RDTSCDriver() :
	running_(false)
{
}

// the driver is NOT unloaded if the class is destroyed because the user might
// want to use it although this controller class is gone out of scope
RDTSCDriver::~RDTSCDriver()
{
}

// start or stop given driver
void RDTSCDriver::controlDriver(const std::string& driverPath, const std::string& driverName, bool load) const
{
	DWORD lastError = 0;

	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSCManager == NULL) throwSysError(GetLastError(), "Unable to open service manager");

	SC_HANDLE hService = CreateService(hSCManager, driverName.c_str(), driverName.c_str(), SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, driverPath.c_str(), NULL, NULL, NULL, NULL, NULL);
	if (!hService)
	{
		lastError = GetLastError();
		if (lastError == ERROR_SERVICE_EXISTS)
		{
			hService = OpenService(hSCManager, driverName.c_str(), SERVICE_ALL_ACCESS);
		}
		else
		{
			CloseServiceHandle(hSCManager);
			throwSysError(lastError, "Error while trying to create RDTSC driver service");
		}
	}

	if (load)
	{
		if (!StartService(hService, 0, NULL)) 
		{
			lastError = GetLastError();
			if (lastError != ERROR_SERVICE_ALREADY_RUNNING)
			{
				CloseServiceHandle(hSCManager);
				CloseServiceHandle(hService);
				throwSysError(lastError, "Error while trying to start RDTSC driver service");
			}
		}
	}
	else
	{
		SERVICE_STATUS ss;
		ControlService(hService, SERVICE_CONTROL_STOP, &ss);
		if (!DeleteService(hService))
		{
			lastError = GetLastError();
			CloseServiceHandle(hSCManager);
			CloseServiceHandle(hService);
			throwSysError(lastError, "Error while trying to stop RDTSC driver");
		}
	}

	CloseServiceHandle(hSCManager);
	CloseServiceHandle(hService);
}

// extracts the driver file and starts it with the given name
// if no name is given, a random name is generated
void RDTSCDriver::startDriver(const ResourceItem& ri, const string& driverName)
{
	if (running_) return;

	string drvName = driverName;
	if (drvName.empty())
	{
		drvName = genRandomDrvName();
	}
	
	char tmpPath[MAX_PATH];
	GetTempPath(MAX_PATH, tmpPath);
	path p(tmpPath);
	p /= drvName + ".sys";
	if (ri.saveDataToFile(p.string()))
	{
		driverPath_ = p.string();
		driverName_ = drvName;
		controlDriver(driverPath_, driverName_, true);
		running_ = true;
	}
	else throw std::runtime_error("Error while trying to save RDTSC driver to file: " + p.string());
}

void RDTSCDriver::stopDriver()
{
	if (!running_) return;

	controlDriver(driverPath_, driverName_, false);
	DeleteFile(driverPath_.c_str());
	running_ = false;
}

// set emulation mode for running driver
bool RDTSCDriver::setMode(RDTSCMode mode, unsigned int param) const
{
	bool retVal = false;
	string rdtscDevice = "\\\\.\\" + driverName_;
	HANDLE hDevice = CreateFile(rdtscDevice.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDevice != INVALID_HANDLE_VALUE)
	{
		DWORD bytesReturned;
		DWORD ioctlCode = mode == constant ? (DWORD)IOCTL_RDTSCEMU_METHOD_ALWAYS_CONST : (DWORD)IOCTL_RDTSCEMU_METHOD_INCREASING;
		if (DeviceIoControl(hDevice, ioctlCode, &param, sizeof(unsigned int), NULL, 0, &bytesReturned, NULL))
		{
			retVal = true;
		}
		CloseHandle(hDevice);
	}

	return retVal;
}

string RDTSCDriver::genRandomDrvName() const
{
	ostringstream oss;
	LARGE_INTEGER seed;
	if (QueryPerformanceCounter(&seed))
	{
		oss << hex << seed.HighPart << seed.LowPart;
	}
	else
	{
		// fallback
		oss << hex << GetTickCount();
	}

	return oss.str();
}

void RDTSCDriver::throwSysError(unsigned int lastError, const std::string& msg) const
{
	ostringstream oss;
	oss << msg << ", system error code was: " << lastError;
	throw std::runtime_error(oss.str());
}