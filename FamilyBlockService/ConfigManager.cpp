#include "ConfigManager.h"
#include <fstream>
#include <stdexcept>
#include <Windows.h>
#include <mutex>
#include "json.hpp"
#include "Logger.h"
using json = nlohmann::json;

// ---------- String helpers (ASCII-safe for exe names) ----------

static std::wstring toWString(const std::string& s) {
    return std::wstring(s.begin(), s.end());
}

static std::string toString(const std::wstring& ws) {
    return std::string(ws.begin(), ws.end());
}

static std::mutex configMutex;

// ---------- Atomic file write helper ----------

static void atomicWrite(const std::string& path, const std::string& data) {
    const std::string tmpPath = path + ".tmp";

    std::ofstream out(tmpPath, std::ios::trunc);
    if (!out.is_open()) {
        log("FAILED: cannot open temp file");
        throw std::runtime_error("Failed to open temp config file");
    }
    log("file opened");
    out << data;
    out.flush();
    out.close();
    log("file closed");
    if (!MoveFileExA(
        tmpPath.c_str(),
        path.c_str(),
        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        log("FAILED: MoveFileExA");
        DeleteFileA(tmpPath.c_str());
        throw std::runtime_error("Failed to replace config file atomically");
    }
    log("atomicWrite success");
}

// ---------- Load config ----------

TimeConfig ConfigManager::load(const std::string& path) {

    std::lock_guard<std::mutex> lock(configMutex);

    TimeConfig cfg;
    json j;

    {
        std::ifstream f(path);
        if (!f) return cfg;

        try {
            f >> j;
        }
        catch (const json::exception& e) {
            return cfg;
        }
    }

    // Core state
    cfg.id = j.value("id", "");
    cfg.secret = j.value("secret", "");
    cfg.remainingMinutes = j.value("remainingMinutes", cfg.remainingMinutes);
    cfg.minutesToReduce = j.value("minutesToReduce", 0);
    cfg.isRunning = j.value("isRunning", false);

    // Blocked apps (continuous enforcement)
    if (j.contains("blockedApps") && j["blockedApps"].is_array()) {
        for (const auto& app : j["blockedApps"]) {
            cfg.blockedApps.insert(toWString(app.get<std::string>()));
        }
    }

    // Blocked browsers (one-time termination)
    if (j.contains("blockedBrowserApps") && j["blockedBrowserApps"].is_array()) {
        for (const auto& app : j["blockedBrowserApps"]) {
            cfg.blockedBrowserApps.insert(toWString(app.get<std::string>()));
        }
    }

    // Blocked websites (hosts file)
    if (j.contains("blockedWebsites") && j["blockedWebsites"].is_array()) {
        for (const auto& website : j["blockedWebsites"]) {
            cfg.blockedWebsites.insert(website.get<std::string>());
        }
    }

    return cfg;
}

// ---------- Save full config (use sparingly) ----------

void ConfigManager::save(const TimeConfig& cfg, const std::string& path) {

    std::lock_guard<std::mutex> lock(configMutex);
    json j;

    j["id"] = cfg.id;
    j["secret"] = cfg.secret;

    j["remainingMinutes"] = cfg.remainingMinutes;
    j["minutesToReduce"] = cfg.minutesToReduce;
    j["isRunning"] = cfg.isRunning;

    j["blockedApps"] = json::array();
    for (const auto& app : cfg.blockedApps) {
        j["blockedApps"].push_back(toString(app));
    }

    j["blockedBrowserApps"] = json::array();
    for (const auto& app : cfg.blockedBrowserApps) {
        j["blockedBrowserApps"].push_back(toString(app));
    }

    j["blockedWebsites"] = json::array();
    for (const auto& website : cfg.blockedWebsites) {
        j["blockedWebsites"].push_back(website);
    }

    atomicWrite(path, j.dump(4));
}

// ---------- Update time only (safe + minimal) ----------

void ConfigManager::saveNewTime(const TimeConfig& cfg, const std::string& path) {

    std::lock_guard<std::mutex> lock(configMutex);
    json j;

    {
        std::ifstream f(path);
        if (f.good()) {
            try {
                f >> j;
            }
            catch (const std::exception&) {
                j = json::object();
            }
        }
    }
    

    j["remainingMinutes"] = cfg.remainingMinutes;
    j["minutesToReduce"] = cfg.minutesToReduce;

    atomicWrite(path, j.dump(4));
}

// ---------- Update running state only ----------

void ConfigManager::saveIsRunning(const TimeConfig& cfg, const std::string& path) {

    std::lock_guard<std::mutex> lock(configMutex);
    json j;

    {
        std::ifstream f(path);
        if (f.good()) {
            try {
                f >> j;
            }
            catch (const std::exception&) {
                j = json::object();
            }
        }
    }
    

    j["isRunning"] = cfg.isRunning;

    atomicWrite(path, j.dump(4));
}
