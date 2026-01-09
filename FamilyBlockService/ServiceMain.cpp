#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>

#include <windows.h>   // ONLY after winsock2


#include <string>
#include <iostream>
#include "Controller.h"
#include "ProcessMonitor.h"
#include "HostsBlocker.h"
#include "ConfigManager.h"
#include "BackendClient.h"


static const char* SERVICE_NAME = "FamilyBlockService";

// Global service state
SERVICE_STATUS          g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE   g_StatusHandle = nullptr;
Controller* g_Controller = nullptr;

// Helper: update service status for SCM
void SetServiceStatusState(DWORD state, DWORD win32ExitCode = NO_ERROR, DWORD waitHint = 0)
{
    g_ServiceStatus.dwCurrentState = state;
    g_ServiceStatus.dwWin32ExitCode = win32ExitCode;
    g_ServiceStatus.dwWaitHint = waitHint;

    if (state == SERVICE_START_PENDING)
        g_ServiceStatus.dwControlsAccepted = 0;
    else
        g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;

    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
}

// Control handler: called by SCM when service is stopped/shutdown
void WINAPI ServiceCtrlHandler(DWORD ctrl)
{
    switch (ctrl)
    {
    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:
        if (g_Controller) {
            g_Controller->requestStop();
        }
        SetServiceStatusState(SERVICE_STOP_PENDING);
        break;
    default:
        break;
    }
}

// Service entry point: called by SCM when starting the service
void WINAPI ServiceMain(DWORD argc, LPSTR* argv)
{
    g_StatusHandle = RegisterServiceCtrlHandlerA(SERVICE_NAME, ServiceCtrlHandler);
    if (!g_StatusHandle) {
        return;
    }

    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwServiceSpecificExitCode = 0;

    SetServiceStatusState(SERVICE_START_PENDING);

    std::string host = "192.168.1.169"; // or your backend domain
    int port = 8081;                    // whatever your Spring Boot server listens on
    bool useSSL = false;                 // true if HTTPS, false if HTTP

    // Your agent identity
    std::string deviceId = "a1";
    std::string deviceSecret = "mySecret123";

    // Create the client
    BackendClient bc(host, port, useSSL, deviceId, deviceSecret);


    // Create dependencies
    ProcessMonitor pm;
    HostsBlocker  hb;
    ConfigManager cm;
    

    Controller controller(pm, hb, cm, bc);
    g_Controller = &controller;

    SetServiceStatusState(SERVICE_RUNNING);

    controller.runService();

    SetServiceStatusState(SERVICE_STOPPED);
}

// Standard entry point for a service executable
int main()
{
    SERVICE_TABLE_ENTRYA serviceTable[] =
    {
        { const_cast<LPSTR>(SERVICE_NAME), reinterpret_cast<LPSERVICE_MAIN_FUNCTIONA>(ServiceMain) },
        { nullptr, nullptr }
    };

    StartServiceCtrlDispatcherA(serviceTable);

    return 0;
}
