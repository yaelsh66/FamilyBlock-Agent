#include "EnvConfig.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace {

std::string trim(const std::string& value) {
    const auto start = value.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }

    const auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(start, end - start + 1);
}

bool parseBool(const std::string& value) {
    std::string lower = value;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
        });

    return lower == "1" || lower == "true" || lower == "yes" || lower == "on";
}

std::optional<BackendEndpoint> parseBackendUrl(const std::string& rawUrl) {
    std::string url = trim(rawUrl);
    if (url.empty()) {
        return std::nullopt;
    }

    BackendEndpoint endpoint;
    const bool https = url.rfind("https://", 0) == 0;
    const bool http = url.rfind("http://", 0) == 0;

    if (!https && !http) {
        return std::nullopt;
    }

    endpoint.useSSL = https;
    url = url.substr(https ? 8 : 7);

    const auto pathPos = url.find('/');
    if (pathPos != std::string::npos) {
        url = url.substr(0, pathPos);
    }

    const auto colonPos = url.find(':');
    if (colonPos == std::string::npos) {
        endpoint.host = url;
        endpoint.port = endpoint.useSSL ? 443 : 80;
        return endpoint;
    }

    endpoint.host = url.substr(0, colonPos);
    try {
        endpoint.port = std::stoi(url.substr(colonPos + 1));
    }
    catch (const std::exception&) {
        return std::nullopt;
    }

    return endpoint;
}

} // namespace

std::string GetEnvVar(const char* name, const std::string& defaultValue) {
    char buffer[2048];
    const DWORD length = GetEnvironmentVariableA(name, buffer, static_cast<DWORD>(sizeof(buffer)));
    if (length == 0 || length >= sizeof(buffer)) {
        return defaultValue;
    }

    return std::string(buffer, length);
}

std::optional<BackendEndpoint> ResolveBackendEndpoint() {
    const std::string backendUrl = GetEnvVar("FAMILYBLOCK_BACKEND_URL");
    if (!backendUrl.empty()) {
        if (auto parsed = parseBackendUrl(backendUrl)) {
            return parsed;
        }
    }

    const std::string host = GetEnvVar("BACKEND_HOST", "localhost");
    const std::string portValue = GetEnvVar("BACKEND_PORT", "8081");
    const std::string sslValue = GetEnvVar("BACKEND_USE_SSL", "false");

    BackendEndpoint endpoint;
    endpoint.host = host;

    try {
        endpoint.port = std::stoi(portValue);
    }
    catch (const std::exception&) {
        endpoint.port = 8081;
    }

    endpoint.useSSL = parseBool(sslValue);
    return endpoint;
}

std::string ResolveConfigPath() {
    const std::string configuredPath = GetEnvVar("FAMILYBLOCK_CONFIG_PATH");
    if (!configuredPath.empty()) {
        return configuredPath;
    }

    return "C:\\FamilyBlockService\\config.json";
}
