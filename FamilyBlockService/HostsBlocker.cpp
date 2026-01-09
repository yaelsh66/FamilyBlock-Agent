#include "HostsBlocker.h"
#include <unordered_set>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <Windows.h>
#include "Logger.h"

static const char* BLOCK_START = "# Start FamilyBlock";
static const char* BLOCK_END = "# End FamilyBlock";

void HostsBlocker::setBlockedWebsites(
    const std::unordered_set<std::string>& blockedWebsitesSet) {
    websiteNames = blockedWebsitesSet;
    log("websites set");
}

std::string HostsBlocker::getHostsPath() {
    char systemDir[MAX_PATH];
    GetSystemDirectoryA(systemDir, MAX_PATH);
    return std::string(systemDir) + "\\drivers\\etc\\hosts";
}

void HostsBlocker::applyBlock() {
    // Always remove existing block first (idempotent)
    // removeBlock();

    const std::string hostsPath = getHostsPath();
    const std::string tempPath = hostsPath + ".tmp";

    std::ifstream inFile(hostsPath);
    if (!inFile.is_open()) {
        log("Failed to open hosts file for reading");
        throw std::runtime_error("Failed to open hosts file for reading");
    }

    std::stringstream buffer;
    buffer << inFile.rdbuf();
    inFile.close();

    buffer << "\n" << BLOCK_START << "\n";
    for (const auto& site : websiteNames) {
        buffer << "127.0.0.1 " << site << "\n";
    }
    buffer << BLOCK_END << "\n";

    std::ofstream outFile(tempPath, std::ios::trunc);
    if (!outFile.is_open()) {
        log("Failed to open temp hosts file");
        throw std::runtime_error("Failed to open temp hosts file");
    }

    outFile << buffer.str();
    outFile.close();

    // Atomic replace
    if (!MoveFileExA(
        tempPath.c_str(),
        hostsPath.c_str(),
        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        DeleteFileA(tempPath.c_str());
        DWORD err = GetLastError();
        log("MoveFileEx failed. Error code: " + std::to_string(err));

        log("Failed to replace hosts file");
        throw std::runtime_error("Failed to replace hosts file");
    }
    log("apllyBlockDone");
}

static std::string trim(const std::string& s) {
    auto start = s.begin();
    while (start != s.end() && std::isspace(*start)) ++start;

    auto end = s.end();
    do {
        --end;
    } while (std::distance(start, end) >= 0 && std::isspace(*end));

    return std::string(start, end + 1);
}

void HostsBlocker::removeBlock() {
    const std::string hostsPath = getHostsPath();
    const std::string tempPath = hostsPath + ".tmp";

    std::ifstream inFile(hostsPath);
    if (!inFile.is_open()) {
        log("removeBlock - Failed to open hosts file for reading");
        throw std::runtime_error("Failed to open hosts file for reading");
    }

    std::stringstream buffer;
    std::string line;
    bool skip = false;

    while (std::getline(inFile, line)) {
        std::string trimmed = trim(line);

        // check start marker
        if (trimmed == BLOCK_START) {
        
            skip = true;
            continue;
        }
        if (trimmed == BLOCK_END) {
            skip = false;
            continue;
        }
        if (!skip) {
            buffer << line << "\n";
        }
    }
    inFile.close();

    std::ofstream outFile(tempPath, std::ios::trunc);
    if (!outFile.is_open()) {
        log("removeBlock - Failed to open temp hosts file");
        throw std::runtime_error("Failed to open temp hosts file");
    }

    outFile << buffer.str();
    outFile.close();

    if (!MoveFileExA(
        tempPath.c_str(),
        hostsPath.c_str(),
        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        DeleteFileA(tempPath.c_str());
        log("removeBlock - Failed to restore hosts file");
        throw std::runtime_error("Failed to restore hosts file");
    }
    log("removeBlockDone");
}
