#pragma once

// log window which contains all anti-debugging API events

#include <iostream>

class IDAStealthLogWindow
{
public:

	~IDAStealthLogWindow() {}

	static IDAStealthLogWindow& getInstance()
	{
		static IDAStealthLogWindow instance_;
		return instance_;
	}

	void show();
	void addLogEntry(const std::string& entry);

private:

	IDAStealthLogWindow();

	IDAStealthLogWindow(IDAStealthLogWindow const&);
	IDAStealthLogWindow& operator=(IDAStealthLogWindow const&);
};
