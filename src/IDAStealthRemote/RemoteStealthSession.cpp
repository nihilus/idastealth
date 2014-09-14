#include <boost/filesystem.hpp>
#include <fstream>
#include "RemoteStealthConnection.h"
#include "RemoteStealthSession.h"
#include "resource.h"
#include <Windows.h>

namespace
{
	std::string StealthDllFileName = "HideDebugger.dll";
}

RemoteStealth::RemoteStealthSession::RemoteStealthSession()
{
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	boost::filesystem::path p = buffer;
	p.remove_leaf();
	p /= StealthDllFileName;
	stealthDll_ = p.string();
}

void RemoteStealth::RemoteStealthSession::handleDbgAttach(const RSProtocolItem& item)
{
	std::string configFile = serializeConfig(item.serializedConfigFile);
	StealthSession::handleDbgAttach(item.processID, configFile, item.profile);
}

void RemoteStealth::RemoteStealthSession::handleProcessStart(const RSProtocolItem& item)
{
	std::string configFile = serializeConfig(item.serializedConfigFile);
	StealthSession::handleProcessStart(item.processID, item.baseAddress, configFile, item.profile);
}

// serialize config to config file and return path of file
std::string RemoteStealth::RemoteStealthSession::serializeConfig(const std::string& configStr)
{
	using namespace std;

	char tmpPath[MAX_PATH];
	char tmpFileName[MAX_PATH];

	GetTempPath(MAX_PATH, tmpPath);
	if (!GetTempFileName(tmpPath, "h4x0r", 0, tmpFileName))
		throw runtime_error("Error while trying to serialize configuration to file: unable to get path for temp file");

	ofstream ofs(tmpFileName);
	ofs.write(configStr.c_str(), configStr.length());
	cout << "saving config to file: " << tmpFileName << endl;
	return string(tmpFileName);
}

void RemoteStealth::RemoteStealthSession::logString(const std::string& str)
{
	std::cout << str << std::endl;
}

ResourceItem RemoteStealth::RemoteStealthSession::getRDTSCDriverResource()
{
	return ResourceItem(GetModuleHandle(NULL), IDR_RDTSC, "DRV");
}

ResourceItem RemoteStealth::RemoteStealthSession::getStealthDriverResource()
{
	return ResourceItem(GetModuleHandle(NULL), IDR_STEALTH, "DRV");
}

std::string RemoteStealth::RemoteStealthSession::getStealthDllPath()
{
	return stealthDll_;
}