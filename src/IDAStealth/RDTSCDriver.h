#pragma once

#include <iostream>
#include <RDTSCEmu/driver/RDTSCEmu.h>
#include "ResourceItem.h"

class RDTSCDriver
{
public:

	RDTSCDriver();
	~RDTSCDriver();
	void startDriver(const ResourceItem& ri, const std::string& driverName);
	void stopDriver();
	bool setMode(RDTSCMode mode, unsigned int param) const;
	std::string getDriverPath() const { return driverPath_; }
	bool isRunning() const { return running_; }

private:
	
	bool running_;
	std::string driverPath_;
	std::string driverName_;
	std::string genRandomDrvName() const;
	void controlDriver(const std::string& driverPath, const std::string& driverName, bool load) const;
	void throwSysError(unsigned int lastError, const std::string& msg) const;
};
