#pragma once

// singleton config class to serialize all GUI options to an ini file

#include <iostream>
#include <RDTSCEmu/driver/RDTSCEmu.h>
#include <string>
#include <vector>

static const char* PEBIsDebuggedStr = "PEBIsDebugged";
static const char* NtQuerySystemInfoStr = "NtQuerySystemInformation";
static const char* NtQueryInfoProcessStr = "NtQueryInformationProcess";
static const char* NtGlobalFlagStr = "NtGlobalFlag";
static const char* HeapFlagsStr = "HeapFlags";
static const char* NtSetInfoThreadStr = "NtSetInformationThread";
static const char* NtQueryObjectStr = "NtQueryObject";
static const char* OutputDbgStr = "OutputDebugString";
static const char* NtCloseStr = "NtClose";
static const char* EnableDbgStartStr = "EnableDbgStart";
static const char* EnableDbgAttachStr = "EnableDbgAttach";
static const char* GetTickCountStr = "GetTickCount";
static const char* RtlGetNtGlobalFlagsStr = "RtlGetNtGlobalFlags";
static const char* BlockInputStr = "BlockInput";
static const char* SuspendThreadStr = "SuspendThread";
static const char* DbgPrintExceptionStr = "DbgPrintException";
static const char* OpenProcessStr = "OpenProcess";
static const char* SwitchDesktopStr = "SwitchDesktop";
static const char* FakeParentProcessStr = "FakeParentProcess";
static const char* ProtectDRsStr = "ProtectDRs";
static const char* NtYieldExecutionStr = "NtYieldExecution";
static const char* HideIDAWindowStr = "HideIDAWindow";
static const char* HideIDAProcessStr = "HideIDAProcess";
static const char* GetVersionStr = "GetVersion";
static const char* NtTerminateStr = "NtTerminate";
static const char* KillAntiAttachStr = "KillAntiAttach";
static const char* RDTSCEmulationStr = "RDTSCEmulation";
static const char* RDTSCUnloadDrvStr = "UnloadRDTSCDriver";
static const char* RandRDTSCNameStr = "RandRDTSCName";
static const char* PassUnknownExceptionsStr = "PassUnknownExceptions";
static const char* InlinePatchingMethodStr = "InlinePatchingMethod";
static const char* RemoteTCPPortStr = "RemoteTCPPort";
static const char* StealthDriverStr = "StealthDriver";
static const char* StealthNtSetInfoThreadStr = "StealthNtSetInformationThread";
static const char* StealthDriverUnloadStr = "UnloadStealthDriver";
static const char* StealthNtQueryInfoProcessStr = "StealthNtQueryInformationProcess";
static const char* HaltInSEHHandlerStr = "HaltInSEHHandler";
static const char* HaltAfterSEHHandlerStr = "HaltAfterSEHHandler";
static const char* LogSEHStr = "LogSEH";

enum InlinePatching { AutoSelect, ForceAbsolute };

class HideDebuggerConfig
{
public:

	static HideDebuggerConfig& getInstance()
	{
		static HideDebuggerConfig instance_;
		return instance_;
	}

	~HideDebuggerConfig() {};

	static std::string getDefaultConfigFile();
	void setConfigFile(const std::string& configFile);

	bool delProfile(const std::string& profileName);
	std::vector<std::string> getProfiles() const;
	
	void switchToProfile(const std::string& profileName);
	std::string getCurrentProfile() const;

	bool getPEBIsDebugged() const;
	void setPEBIsDebugged(bool value) const;

	bool getNtQueryInfoProcess() const;
	void setNtQueryInfoProcess(bool value) const;

	bool getNtQuerySystemInfo() const;
	void setNtQuerySystemInfo(bool value) const;

	bool getNtGlobalFlag() const;
	void setNtGlobalFlag(bool value) const;

	bool getHeapFlags() const;
	void setHeapFlags(bool value) const;

	bool getNtSetInfoThread() const;
	void setNtSetInfoThread(bool value) const;

	bool getNtQueryObject() const;
	void setNtQueryObject(bool value) const;

	bool getOutputDbgStr() const;
	void setOutputDbgStr(bool value) const;

	bool getNtClose() const;
	void setNtClose(bool value) const;

	bool getEnableDbgStart() const;
	void setEnableDbgStart(bool value) const;

	bool getEnableDbgAttach() const;
	void setEnableDbgAttach(bool value) const;

	bool getGetTickCount() const;
	void setGetTickCount(bool value) const;

	int getGetTickCountDelta() const;
	void setGetTickCountDelta(int value) const;

	bool getRtlGetNtGlobalFlags() const;
	void setRtlGetNtGlobalFlags(bool value) const;

	bool getBlockInput() const;
	void setBlockInput(bool value) const;

	bool getSuspendThread() const;
	void setSuspendThread(bool value) const;

	bool getDbgPrintException() const;
	void setDbgPrintException(bool value) const;

	bool getOpenProcess() const;
	void setOpenProcess(bool value) const;

	bool getSwitchDesktop() const;
	void setSwitchDesktop(bool value) const;

	bool getFakeParentProcess() const;
	void setFakeParentProcess(bool value) const;

	bool getProtectDRs() const;
	void setProtectDRs(bool value) const;

	bool getNtYieldExecution() const;
	void setNtYieldExecution(bool value) const;

	bool getHideIDAWindow() const;
	void setHideIDAWindow(bool value) const;

	bool getHideIDAProcess() const;
	void setHideIDAProcess(bool value) const;

	bool getGetVersion() const;
	void setGetVersion(bool value) const;

	bool getNtTerminate() const;
	void setNtTerminate(bool value) const;

	bool getKillAntiAttach() const;
	void setKillAntiAttach(bool value) const;

	bool getUseRDTSC() const;
	void setUseRDTSC(bool value) const;

	bool getRandRDTSCName() const;
	void setRandRDTSCName(bool value) const;

	RDTSCMode getRDTSCMode() const;
	void setRDTSCMode(RDTSCMode value) const;

	int getRDTSCIncrDelta() const;
	void setRDTSCIncrDelta(int value) const;

	bool getUnloadRDTSC() const;
	void setUnloadRDTSC(bool value) const;

	bool getPassExceptions() const;
	void setPassExceptions(bool value);

	InlinePatching getInlinePatching() const;
	void setInlinePatching(InlinePatching value) const;

	int getRemoteTCPPort() const;
	void setRemoteTCPPort(int value) const;

	bool getUseStealthDriver() const;
	void setUseStealthDriver(bool value);

	bool getStealthNtSetInfoThread() const;
	void setStealthNtSetInfoThread(bool value) const;

	bool getUnloadStealth() const;
	void setUnloadStealth(bool value) const;

	bool getStealthNtQueryInfoProcess() const;
	void setStealthNtQueryInfoProcess(bool value);

	bool getHaltInSEH() const;
	void setHaltInSEH(bool value);

	bool getHaltAfterSEH() const;
	void setHaltAfterSEH(bool value);

	bool getLogSEH() const;
	void setLogSEH(bool value);

private:

	HideDebuggerConfig();

	HideDebuggerConfig(HideDebuggerConfig const&);
	HideDebuggerConfig& operator=(HideDebuggerConfig const&);

	bool isSettingEnabled(const std::string& settingStr) const;
	void toggleSetting(const std::string& settingStr, bool value) const;

	template <typename T>
	void readFromIni(const std::string& setting, const std::string& attribute, T& result) const;

	template <typename T>
	void writeToIni(const std::string& setting, const std::string& attribute, const T& value) const;

	bool fileExists(const std::string& fileName) const;

	std::string configFile_;
	std::string currentProfile_;
};