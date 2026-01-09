#pragma once
#include <unordered_set>
#include <string>


struct TimeConfig {
	std::unordered_set<std::wstring> blockedApps;
	std::unordered_set<std::string> blockedWebsites;
	std::unordered_set<std::wstring> blockedBrowserApps;

	std::string id;
	std::string secret;
	int minutesToReduce = 0;
	bool isRunning = true;
	int remainingMinutes = 6;
};

class ConfigManager
{
public:
	TimeConfig load(const std::string& path);
	void save(const TimeConfig& cfg, const std::string& path);
	void saveNewTime(const TimeConfig& cfg, const std::string& path);
	void saveIsRunning(const TimeConfig& cfg, const std::string& path);
};

