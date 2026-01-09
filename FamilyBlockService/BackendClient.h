#pragma once
#include <string>
#include <iostream>
#include <optional>

#include "httplib.h"     // cpp-httplib
#include "ConfigManager.h" // for TimeConfig

class BackendClient
{
private:
    std::string host;      // Example: "api.myserver.com"
    int port;              // Example: 443 (HTTPS)
    bool useSSL;           // true = HTTPS client, false = HTTP client

    std::string deviceId;      // Identifies THIS agent installation
    std::string deviceSecret;  // Authenticates this agent

    std::unique_ptr<httplib::Client> client;
public:
    BackendClient(const std::string& host, int port, bool useSSL, const std::string& deviceId, const std::string& deviceSecret);

    std::optional<TimeConfig> heartbeat(const TimeConfig& currentCfg);

};

