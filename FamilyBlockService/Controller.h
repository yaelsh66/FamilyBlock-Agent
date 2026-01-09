#pragma once
#include <string>
#include <atomic>
#include <thread>
#include <iostream>
#include <filesystem>
#include <chrono>
#include "ProcessMonitor.h"
#include "ConfigManager.h"
#include "HostsBlocker.h"
#include "BackendClient.h"


class Controller
{
private:
	ProcessMonitor processManager;
	ConfigManager configManager;
	HostsBlocker hostsBlocker;
	BackendClient& backendClient;

	std::string configPath;
	TimeConfig  cfg{};          // loaded from JSON
	int secondsSinceLastReload = 0; // counts seconds to turn into minutes for reloud config
	int secondsCounter = 0;    // counts seconds to turn into minutes
	std::atomic<bool> stopRequested{ false };

public:
	Controller(ProcessMonitor& procm, HostsBlocker& hostB, ConfigManager& configm, BackendClient& backend);
	

	// Main loop: runs until requestStop() is called
	void runService();

	// Signal the loop to exit (used by service control handler later)
	void requestStop();

private:
	void applyBackendConfig(const TimeConfig& backendCfg);
	void reloadConfig();               // reloads JSON and detects mode change
	void enterBlockedMode();
	void enterAllowedMode();

	void tickBlockedMode();            // called every second in blocked mode
	void tickAllowedMode();            // called every second in allowed mode

};

