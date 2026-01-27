#include "BackendClient.h"
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include "json.hpp"
#include <httplib.h>
#include "Logger.h"

static std::wstring toWString(const std::string& s) {
    return std::wstring(s.begin(), s.end());
}

BackendClient::BackendClient(
    const std::string& host,
    int port,
    bool useSSL,
    const std::string& deviceId,
    const std::string& deviceSecret)
    : host(host),
    port(port),
    useSSL(useSSL),
    deviceId(deviceId),
    deviceSecret(deviceSecret) {
    // Create HTTP / HTTPS client ONCE
    if (useSSL) {
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        client = std::make_unique<httplib::SSLClient>(host, port);
#else
        std::cerr << "[BackendClient] SSL requested but not supported\n";
        client = nullptr;
#endif
    }
    else {
        client = std::make_unique<httplib::Client>(host, port);
    }
    if (client) {
        client->set_connection_timeout(5);
        client->set_read_timeout(5, 0);
        client->set_write_timeout(5, 0);
    }
}

std::optional<TimeConfig>
BackendClient::heartbeat(const TimeConfig& currentCfg) {
    using json = nlohmann::json;
    log("heartbeat");
    if (!client) {
        std::cerr << "[BackendClient] HTTP client not initialized\n";
        return std::nullopt;
    }

    try {
        log("try");
        // ---------------------------
        // Build JSON request
        // ---------------------------
        json request;
        request["deviceId"] = deviceId;
        request["deviceSecret"] = deviceSecret;
        request["isRunning"] = currentCfg.isRunning;
        request["minutesToReduce"] = currentCfg.minutesToReduce;

        const std::string body = request.dump();

        // ---------------------------
        // Create HTTP or HTTPS client
        // ---------------------------
        

       
        log("Before Post");
        auto res = client->Post(
            "/agent/heartbeat",
            body,
            "application/json");

        if (!res) {
            std::cerr << "[BackendClient] Network error or no response\n";
            return std::nullopt;
        }

        log("After Post");

        if (res->status != 200) {
            std::cerr << "[BackendClient] Backend returned status "
                << res->status << "\n";
            return std::nullopt;
        }

        // ---------------------------
        // Parse JSON response
        // ---------------------------
        json reply;
        try {
            reply = json::parse(res->body);
        }
        catch (const std::exception& ex) {
            std::cerr << "[BackendClient] Invalid JSON reply: "
                << ex.what() << "\n";
            return std::nullopt;
        }

        // ---------------------------
        // Build updated config
        // Backend is authoritative
        // ---------------------------
        TimeConfig updated;

        updated.isRunning =
            reply.value("isRunning", currentCfg.isRunning);

        updated.remainingMinutes =
            reply.value("availableMinutes",
                currentCfg.remainingMinutes);

        updated.blockedApps.clear();
        if (reply.contains("blockedApps") &&
            reply["blockedApps"].is_array()) {
            for (const auto& app : reply["blockedApps"]) {
                updated.blockedApps.insert(
                    toWString(app.get<std::string>()));
            }
        }

        updated.blockedBrowserApps.clear();
        if (reply.contains("blockedBrowserApps") &&
            reply["blockedBrowserApps"].is_array()) {
            for (const auto& app : reply["blockedBrowserApps"]) {
                updated.blockedBrowserApps.insert(
                    toWString(app.get<std::string>()));
            }
        }

        updated.blockedWebsites.clear();
        if (reply.contains("blockedWebsites") &&
            reply["blockedWebsites"].is_array()) {
            for (const auto& site : reply["blockedWebsites"]) {
                updated.blockedWebsites.insert(
                    site.get<std::string>());
            }
        }

        return updated;
    }
    catch (const std::exception& ex) {
        // Absolute safety: never crash the service
        std::cerr << "[BackendClient] Exception: "
            << ex.what() << "\n";
        return std::nullopt;
    }
}
