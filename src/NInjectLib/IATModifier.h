#pragma once

#include <iostream>
#include <vector>
#include "Process.h"

class IATModifier
{
public:

	IATModifier(const Process& process);
	~IATModifier();
	std::vector<IMAGE_IMPORT_DESCRIPTOR> readImportTable();
	void setImageBase(uintptr_t address);
	void writeIAT(const std::vector<std::string>& dlls);
	void writeIAT(const std::string& dll);
	IMAGE_NT_HEADERS readNTHeaders() const;

private:

	DWORD pad(DWORD val, DWORD amount) { return (val+amount) & ~amount; };
	DWORD padToDword(DWORD val) { return pad(val, 3); };
	void* allocateMemAboveBase(void* baseAddress, size_t size);
	DWORD determineIIDSize(PIMAGE_IMPORT_DESCRIPTOR importDescriptorTableAddress);
	Process process_;
	PIMAGE_IMPORT_DESCRIPTOR importDescrTblAddr_;
	uintptr_t ntHeadersAddr_;
	unsigned int importDescrTblSize_;
};

// general exception for this class
class IATModifierException : public std::runtime_error
{
public:
	IATModifierException::IATModifierException(const std::string& msg) : std::runtime_error(msg) {};
};

// exception while writing image import descriptors
class WriteIIDException : public std::runtime_error
{
public:
	WriteIIDException::WriteIIDException(const std::string& msg, const MemoryAccessException& e) : std::runtime_error(msg), innerException_(e) {};
	MemoryAccessException innerException() const { return innerException_; };

private:
	MemoryAccessException innerException_;
};