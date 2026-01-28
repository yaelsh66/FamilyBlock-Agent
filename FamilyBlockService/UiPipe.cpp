#include "UiPipe.h"
#include <windows.h>
#include <cstring>

static HANDLE g_hPipe = INVALID_HANDLE_VALUE;
static volatile bool g_uiConnected = false;

// Thread waits for UI to connect
DWORD WINAPI UiPipeThread(LPVOID)
{
    BOOL ok = ConnectNamedPipe(g_hPipe, NULL) ||
        GetLastError() == ERROR_PIPE_CONNECTED;

    if (ok)
        g_uiConnected = true;

    return 0;
}
//{
//    ConnectNamedPipe(g_hPipe, NULL);
//    return 0;
//}

void InitUiPipe()
{
    g_hPipe = CreateNamedPipeW(
        L"\\\\.\\pipe\\MyServicePipe",
        PIPE_ACCESS_OUTBOUND,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        1,
        512,
        512,
        0,
        NULL
    );

    if (g_hPipe == INVALID_HANDLE_VALUE)
        return;

    CreateThread(NULL, 0, UiPipeThread, NULL, 0, NULL);
}

void SendUiMessage(const char* text)
{
    if (!g_uiConnected)
        return;

//    if (g_hPipe == INVALID_HANDLE_VALUE)
//        return;

    DWORD written;
    BOOL ok = WriteFile(
        g_hPipe,
        text,
        (DWORD)strlen(text),
        &written,
        NULL
    );

    if (!ok) {
        // UI likely closed; ignore
        // return;

        // UI closed or pipe broken
        g_uiConnected = false;
        DisconnectNamedPipe(g_hPipe);

        // Prepare for next UI instance
        CreateThread(NULL, 0, UiPipeThread, NULL, 0, NULL);
    }

    // FlushFileBuffers(g_hPipe);
}
