#pragma once
#include <unordered_set>
#include <string>
#include <Windows.h>

class ProcessMonitor
{
private:
	std::unordered_set<std::wstring> appNames;
	std::unordered_set<std::wstring> browserNames;
	
public:
	void scanAndTerminate(const std::unordered_set<std::wstring>& processToKill);
	
	
	
	
};

