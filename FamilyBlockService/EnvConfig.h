#pragma once

#include <optional>
#include <string>

struct BackendEndpoint {
    std::string host;
    int port = 8081;
    bool useSSL = false;
};

std::string GetEnvVar(const char* name, const std::string& defaultValue = "");
std::optional<BackendEndpoint> ResolveBackendEndpoint();
std::string ResolveConfigPath();
