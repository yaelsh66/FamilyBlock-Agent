#include "ProcessMonitor.h"
#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <algorithm>
#include "Logger.h"
// Processes that must NEVER be terminated
static const std::unordered_set<std::wstring> CRITICAL_PROCESSES = {
    L"system",
    L"system idle process",
    L"wininit.exe",
    L"winlogon.exe",
    L"csrss.exe",
    L"services.exe",
    L"lsass.exe",
    L"smss.exe",
    L"explorer.exe" 
};

static std::wstring toLower(const std::wstring& s) {
    std::wstring out = s;
    std::transform(out.begin(), out.end(), out.begin(),
        [](wchar_t c) { return towlower(c); });
    return out;
}

void ProcessMonitor::scanAndTerminate(
    const std::unordered_set<std::wstring>& processToKill) {

    if (processToKill.empty())
        return;

    // PID of this service process (absolute self-protection)
    DWORD selfPid = GetCurrentProcessId();

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "[ProcessMonitor] Failed to create process snapshot\n";
        return;
    }

    PROCESSENTRY32W pe32{};
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    if (!Process32FirstW(snapshot, &pe32)) {
        CloseHandle(snapshot);
        return;
    }

    // Pre-normalize blocked list to lowercase
    std::unordered_set<std::wstring> normalized;
    for (const auto& name : processToKill)
        normalized.insert(toLower(name));

    do {
        // Never kill ourselves (PID check)
        if (pe32.th32ProcessID == selfPid)
            continue;

        std::wstring exeName = toLower(pe32.szExeFile);

        // Never kill critical processes
        if (CRITICAL_PROCESSES.find(exeName) != CRITICAL_PROCESSES.end())
            continue;

        // Never kill ourselves (name check, secondary safety)
        if (exeName == L"familyblockservice.exe")
            continue;

        if (normalized.find(exeName) != normalized.end()) {
            
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE,
                pe32.th32ProcessID);
            if (!hProcess) {
                std::cerr << "[ProcessMonitor] Access denied: "
                    << std::string(exeName.begin(), exeName.end())
                    << "\n";
                continue;
            }

            if (!TerminateProcess(hProcess, 1)) {
                std::cerr << "[ProcessMonitor] Failed to terminate: "
                    << std::string(exeName.begin(), exeName.end())
                    << "\n";
            }

            CloseHandle(hProcess);
        }

    } while (Process32NextW(snapshot, &pe32));

    CloseHandle(snapshot);
}
