#include "DriverControl.h"
#include <Windows.h>
#include <sstream>
#include <boost/filesystem.hpp>

using namespace std;
using namespace boost::filesystem;

DriverControl::DriverControl() :
	running_(false)
{
}

// the driver is NOT unloaded if the class is destroyed because the user might
// want to use it although this controller class has gone out of scope
DriverControl::~DriverControl()
{
}

// start or stop given driver
void DriverControl::controlDriver(const std::string& driverPath, const std::string& driverName, bool load) const
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
			throwSysError(lastError, "Error while trying to create driver service with name: " + driverName);
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
				throwSysError(lastError, "Error while trying to start driver service");
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
			throwSysError(lastError, "Error while trying to stop driver");
		}
	}

	CloseServiceHandle(hSCManager);
	CloseServiceHandle(hService);
}

// extracts the driver file and starts it with the given name
// if no name is given, a random name is generated
void DriverControl::startDriver(const ResourceItem& ri, const string& driverName)
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
	else throw std::runtime_error("Error while trying to save driver to file: " + p.string());
}

void DriverControl::stopDriver()
{
	if (!running_) return;
	controlDriver(driverPath_, driverName_, false);
	DeleteFile(driverPath_.c_str());
	running_ = false;
}

// send IOCTL command to driver
void DriverControl::setMode(unsigned int ioctlCode, void* param, size_t paramSize) const
{
	string device = "\\\\.\\" + driverName_;
	HANDLE hDevice = CreateFile(device.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDevice != INVALID_HANDLE_VALUE)
	{
		DWORD bytesReturned;
		//  DeviceIoControl(hDevice, ioctlCode, &param, sizeof(unsigned int), NULL, 0, &bytesReturned, NULL))
		if (!DeviceIoControl(hDevice, ioctlCode, param, paramSize, NULL, 0, &bytesReturned, NULL))
		{
			DWORD lastErr = GetLastError();
			CloseHandle(hDevice);
			throwSysError(lastErr, "Unable to send IOCTL command to driver: " + driverName_);
		}
		CloseHandle(hDevice);
	}
	else
	{
		DWORD lastErr = GetLastError();
		throwSysError(lastErr, "Unable to open driver object: " + driverName_);
	}
}

string DriverControl::genRandomDrvName() const
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

void DriverControl::throwSysError(unsigned int lastError, const std::string& msg) const
{
	ostringstream oss;
	oss << msg << ", system error code was: " << lastError;
	throw std::runtime_error(oss.str());
}