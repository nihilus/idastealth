#include <boost/filesystem.hpp>
#include "HideDebuggerConfig.h"
#include <IniFileAccess/IniFileAccess.h>
#include <shlwapi.h>
#include <ShlObj.h>
#include <shellapi.h>

namespace
{
	const char* EnabledStr = "enabled";
	const char* ModeStr = "mode";
	const char* ValueStr = "value";

	const char* DefaultConfigFile = "HideDebugger.ini";
	const char* AppPath = "IDAStealth";

	const char* DefaultProfile = "default";
	const char* LastProfile = "last_profile";
}

// get default path for config file and create it if it's not existing
// throw exception on filesystem error
std::string HideDebuggerConfig::getDefaultConfigFile()
{
	char appDataPath[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, (LPSTR)appDataPath)))
	{
		using namespace boost::filesystem;
		path p(appDataPath);
		p /= AppPath;
		if (!exists(p)) create_directory(p);
		p /= DefaultConfigFile;
		return p.string();
	}
	throw std::runtime_error("Failed to retrieve path for default config file");
}

HideDebuggerConfig::HideDebuggerConfig()
{
	configFile_ = getDefaultConfigFile();
	IniFileAccess iniFile(configFile_);
	iniFile.readValue(LastProfile, currentProfile_);
	if (!currentProfile_.size()) currentProfile_ = DefaultProfile;
}

bool HideDebuggerConfig::getPEBIsDebugged() const
{
	return isSettingEnabled(PEBIsDebuggedStr);
}

void HideDebuggerConfig::setPEBIsDebugged(bool value) const
{
	toggleSetting(PEBIsDebuggedStr, value);
}

bool HideDebuggerConfig::getNtQueryInfoProcess() const
{
	return isSettingEnabled(NtQueryInfoProcessStr);
}

void HideDebuggerConfig::setNtQueryInfoProcess(bool value) const
{
	toggleSetting(NtQueryInfoProcessStr, value);
}

bool HideDebuggerConfig::getNtQuerySystemInfo() const
{
	return isSettingEnabled(NtQuerySystemInfoStr);
}

void HideDebuggerConfig::setNtQuerySystemInfo(bool value) const
{
	toggleSetting(NtQuerySystemInfoStr, value);
}

bool HideDebuggerConfig::getNtGlobalFlag() const
{
	return isSettingEnabled(NtGlobalFlagStr);
}

void HideDebuggerConfig::setNtGlobalFlag(bool value) const
{
	toggleSetting(NtGlobalFlagStr, value);
}

bool HideDebuggerConfig::getHeapFlags() const
{
	return isSettingEnabled(HeapFlagsStr);
}
void HideDebuggerConfig::setHeapFlags(bool value) const
{
	toggleSetting(HeapFlagsStr, value);
}

bool HideDebuggerConfig::getNtSetInfoThread() const
{
	return isSettingEnabled(NtSetInfoThreadStr);
}

void HideDebuggerConfig::setNtSetInfoThread(bool value) const
{
	toggleSetting(NtSetInfoThreadStr, value);
}

bool HideDebuggerConfig::getNtQueryObject() const
{
	return isSettingEnabled(NtQueryObjectStr);
}

void HideDebuggerConfig::setNtQueryObject(bool value) const
{
	toggleSetting(NtQueryObjectStr, value);
}

bool HideDebuggerConfig::getOutputDbgStr() const
{
	return isSettingEnabled(OutputDbgStr);
}

void HideDebuggerConfig::setOutputDbgStr(bool value) const
{
	toggleSetting(OutputDbgStr, value);
}

bool HideDebuggerConfig::getNtClose() const
{
	return isSettingEnabled(NtCloseStr);
}

void HideDebuggerConfig::setNtClose(bool value) const
{
	toggleSetting(NtCloseStr, value);
}

bool HideDebuggerConfig::getEnableDbgStart() const
{
	return isSettingEnabled(EnableDbgStartStr);
}

void HideDebuggerConfig::setEnableDbgStart(bool value) const
{
	toggleSetting(EnableDbgStartStr, value);
}

bool HideDebuggerConfig::getEnableDbgAttach() const
{
	return isSettingEnabled(EnableDbgAttachStr);
}

void HideDebuggerConfig::setEnableDbgAttach(bool value) const
{
	toggleSetting(EnableDbgAttachStr, value);
}

bool HideDebuggerConfig::getGetTickCount() const
{
	return isSettingEnabled(GetTickCountStr);
}

void HideDebuggerConfig::setGetTickCount(bool value) const
{
	toggleSetting(GetTickCountStr, value);
}

int HideDebuggerConfig::getGetTickCountDelta() const
{
	int result = 0;
	readFromIni<int>(GetTickCountStr, ValueStr, result);
	return result;
}

void HideDebuggerConfig::setGetTickCountDelta(int value) const
{
	writeToIni<int>(GetTickCountStr, ValueStr, value);
}

bool HideDebuggerConfig::getRtlGetNtGlobalFlags() const
{
	return isSettingEnabled(RtlGetNtGlobalFlagsStr);
}

void HideDebuggerConfig::setRtlGetNtGlobalFlags(bool value) const
{
	toggleSetting(RtlGetNtGlobalFlagsStr, value);
}

bool HideDebuggerConfig::getBlockInput() const
{
	return isSettingEnabled(BlockInputStr);
}

void HideDebuggerConfig::setBlockInput(bool value) const
{
	toggleSetting(BlockInputStr, value);
}

bool HideDebuggerConfig::getSuspendThread() const
{
	return isSettingEnabled(SuspendThreadStr);
}

void HideDebuggerConfig::setSuspendThread(bool value) const
{
	toggleSetting(SuspendThreadStr, value);
}

bool HideDebuggerConfig::getDbgPrintException() const
{
	return isSettingEnabled(DbgPrintExceptionStr);
}

void HideDebuggerConfig::setDbgPrintException(bool value) const
{
	toggleSetting(DbgPrintExceptionStr, value);
}

bool HideDebuggerConfig::getOpenProcess() const
{
	return isSettingEnabled(OpenProcessStr);
}

void HideDebuggerConfig::setOpenProcess(bool value) const
{
	toggleSetting(OpenProcessStr, value);
}

bool HideDebuggerConfig::getSwitchDesktop() const
{
	return isSettingEnabled(SwitchDesktopStr);
}

void HideDebuggerConfig::setSwitchDesktop(bool value) const
{
	toggleSetting(SwitchDesktopStr, value);
}

bool HideDebuggerConfig::getFakeParentProcess() const
{
	return isSettingEnabled(FakeParentProcessStr);
}

void HideDebuggerConfig::setFakeParentProcess(bool value) const
{
	toggleSetting(FakeParentProcessStr, value);
}

bool HideDebuggerConfig::getProtectDRs() const
{
	return isSettingEnabled(ProtectDRsStr);
}

void HideDebuggerConfig::setProtectDRs(bool value) const
{
	toggleSetting(ProtectDRsStr, value);
}

bool HideDebuggerConfig::getNtYieldExecution() const
{
	return isSettingEnabled(NtYieldExecutionStr);
}

void HideDebuggerConfig::setNtYieldExecution(bool value) const
{
	toggleSetting(NtYieldExecutionStr, value);
}

bool HideDebuggerConfig::getHideIDAWindow() const
{
	return isSettingEnabled(HideIDAWindowStr);
}

void HideDebuggerConfig::setHideIDAWindow(bool value) const
{
	toggleSetting(HideIDAWindowStr, value);
}

bool HideDebuggerConfig::getHideIDAProcess() const
{
	return isSettingEnabled(HideIDAProcessStr);
}

void HideDebuggerConfig::setHideIDAProcess(bool value) const
{
	toggleSetting(HideIDAProcessStr, value);
}

bool HideDebuggerConfig::getGetVersion() const
{
	return isSettingEnabled(GetVersionStr);
}

void HideDebuggerConfig::setGetVersion(bool value) const
{
	toggleSetting(GetVersionStr, value);
}

bool HideDebuggerConfig::getNtTerminate() const
{
	return isSettingEnabled(NtTerminateStr);
}

void HideDebuggerConfig::setNtTerminate(bool value) const
{
	toggleSetting(NtTerminateStr, value);
}

bool HideDebuggerConfig::getKillAntiAttach() const
{
	return isSettingEnabled(KillAntiAttachStr);
}

void HideDebuggerConfig::setKillAntiAttach(bool value) const
{
	toggleSetting(KillAntiAttachStr, value);
}

bool HideDebuggerConfig::getUseRDTSC() const
{
	return isSettingEnabled(RDTSCEmulationStr);
}

void HideDebuggerConfig::setUseRDTSC(bool value) const
{
	toggleSetting(RDTSCEmulationStr, value);
}

bool HideDebuggerConfig::getRandRDTSCName() const
{
	return isSettingEnabled(RandRDTSCNameStr);
}

void HideDebuggerConfig::setRandRDTSCName(bool value) const
{
	toggleSetting(RandRDTSCNameStr, value);
}

RDTSCMode HideDebuggerConfig::getRDTSCMode() const
{
	int mode = (int)constant;
	readFromIni<int>(RDTSCEmulationStr, ModeStr, mode);
	return (RDTSCMode)mode;
}

void HideDebuggerConfig::setRDTSCMode(RDTSCMode value) const
{
	writeToIni<int>(RDTSCEmulationStr, ModeStr, (int)value);
}

int HideDebuggerConfig::getRDTSCIncrDelta() const
{
	int result = 0;
	readFromIni<int>(RDTSCEmulationStr, ValueStr, result);
	return result;
}

void HideDebuggerConfig::setRDTSCIncrDelta(int value) const
{
	writeToIni<int>(RDTSCEmulationStr, ValueStr, value);
}

bool HideDebuggerConfig::getUnloadRDTSC() const
{
	return isSettingEnabled(RDTSCUnloadDrvStr);
}

void HideDebuggerConfig::setUnloadRDTSC(bool value) const
{
	toggleSetting(RDTSCUnloadDrvStr, value);
}

bool HideDebuggerConfig::isSettingEnabled(const std::string& settingStr) const
{
	bool result = false;
	readFromIni<bool>(settingStr, EnabledStr, result);
	return result;
}

void HideDebuggerConfig::toggleSetting(const std::string& settingStr, bool value) const
{
	writeToIni<bool>(settingStr, EnabledStr, value);
}

std::string getKeyName(const std::string& setting, const std::string& attribute)
{
	return setting + "_" + attribute;
}

template <typename T>
void HideDebuggerConfig::readFromIni(const std::string& setting, const std::string& attribute, T& result) const
{
	IniFileAccess iniFile(configFile_, currentProfile_);
	iniFile.readValue(getKeyName(setting, attribute), result);
}

template <typename T>
void HideDebuggerConfig::writeToIni(const std::string& setting, const std::string& attribute, const T& value) const
{
	IniFileAccess iniFile(configFile_, currentProfile_);
	if (!iniFile.writeValue(getKeyName(setting, attribute), value))
		throw std::runtime_error("Unable to write settings to config file: " + configFile_);
}

bool HideDebuggerConfig::delProfile(const std::string& profileName)
{
	if (profileName == DefaultProfile) return false;
	IniFileAccess iniFile(configFile_);
	return iniFile.wipeSection(profileName);
}

std::vector<std::string> HideDebuggerConfig::getProfiles() const
{
	IniFileAccess iniFile(configFile_);
	std::vector<std::string> profiles(iniFile.getSections());
	return profiles;
}

void HideDebuggerConfig::switchToProfile(const std::string& profileName)
{
	IniFileAccess iniFile(configFile_);
	if (iniFile.writeValue(LastProfile, profileName))
		currentProfile_ = profileName;
}

void HideDebuggerConfig::setConfigFile(const std::string& configFile)
{
	configFile_ = configFile;
}

std::string HideDebuggerConfig::getCurrentProfile() const
{
	return currentProfile_;
}

bool HideDebuggerConfig::getPassExceptions() const
{
	return isSettingEnabled(PassUnknownExceptionsStr);
}

void HideDebuggerConfig::setPassExceptions(bool value)
{
	toggleSetting(PassUnknownExceptionsStr, value);
}

InlinePatching HideDebuggerConfig::getInlinePatching() const
{
	int method = (int)AutoSelect;
	readFromIni<int>(InlinePatchingMethodStr, ModeStr, method);
	return (InlinePatching)method;
}

void HideDebuggerConfig::setInlinePatching(InlinePatching value) const
{
	writeToIni<int>(InlinePatchingMethodStr, ModeStr, (int)value);
}

int HideDebuggerConfig::getRemoteTCPPort() const
{
	int port = 4242;
	readFromIni<int>(RemoteTCPPortStr, ValueStr, port);
	return port;
}

void HideDebuggerConfig::setRemoteTCPPort(int value) const
{
	writeToIni<int>(RemoteTCPPortStr, ValueStr, value);
}

bool HideDebuggerConfig::getUseStealthDriver() const
{
	return isSettingEnabled(StealthDriverStr);
}

void HideDebuggerConfig::setUseStealthDriver(bool value)
{
	toggleSetting(StealthDriverStr, value);
}

bool HideDebuggerConfig::getStealthNtSetInfoThread() const
{
	return isSettingEnabled(StealthNtSetInfoThreadStr);
}

void HideDebuggerConfig::setStealthNtSetInfoThread(bool value) const
{
	toggleSetting(StealthNtSetInfoThreadStr, value);
}

bool HideDebuggerConfig::getUnloadStealth() const
{
	return isSettingEnabled(StealthDriverUnloadStr);
}

void HideDebuggerConfig::setUnloadStealth(bool value) const
{
	toggleSetting(StealthDriverUnloadStr, value);
}

bool HideDebuggerConfig::getStealthNtQueryInfoProcess() const
{
	return isSettingEnabled(StealthNtQueryInfoProcessStr);
}

void HideDebuggerConfig::setStealthNtQueryInfoProcess(bool value)
{
	toggleSetting(StealthNtQueryInfoProcessStr, value);
}

bool HideDebuggerConfig::getHaltInSEH() const
{
	return isSettingEnabled(HaltInSEHHandlerStr);
}

void HideDebuggerConfig::setHaltInSEH(bool value)
{
	toggleSetting(HaltInSEHHandlerStr, value);
}

bool HideDebuggerConfig::getHaltAfterSEH() const
{
	return isSettingEnabled(HaltAfterSEHHandlerStr);
}

void HideDebuggerConfig::setHaltAfterSEH(bool value)
{
	toggleSetting(HaltAfterSEHHandlerStr, value);
}

bool HideDebuggerConfig::getLogSEH() const
{
	return isSettingEnabled(LogSEHStr);
}

void HideDebuggerConfig::setLogSEH(bool value)
{
	toggleSetting(LogSEHStr, value);
}