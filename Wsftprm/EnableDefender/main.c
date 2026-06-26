#include <windows.h>
#include <stdio.h>

#define SERVICE_WIN_DEFEND    L"WinDefend"
#define SERVICE_NIS           L"WdNisSvc"
#define SERVICE_SENSE         L"Sense"
#define SERVICE_MPKSLDRV      L"MpKslDrv"
#define SERVICE_WDBOOT        L"WdBoot"
#define SERVICE_WDFILTER      L"WdFilter"
#define SERVICE_WDNIS         L"WdNisDrv"
#define SERVICE_SECCORE       L"SecurityHealthService"

typedef struct {
    LPCWSTR szName;
    LPCWSTR szDisplay;
} SERVICE_ENTRY;

SERVICE_ENTRY g_Services[] = {
    { SERVICE_WIN_DEFEND,   L"Windows Defender Antivirus" },
    { SERVICE_NIS,          L"Network Inspection System" },
    { SERVICE_SENSE,        L"Windows Defender Advanced Threat Protection" },
    { SERVICE_MPKSLDRV,     L"Microsoft Protection Kernel" },
    { SERVICE_WDBOOT,       L"Windows Defender Boot Driver" },
    { SERVICE_WDFILTER,     L"Windows Defender Filter Driver" },
    { SERVICE_WDNIS,        L"Windows Defender Network Inspection Driver" },
    { SERVICE_SECCORE,      L"Security Health Service" },
};

BOOL StartDefenderService(SC_HANDLE hSCM, LPCWSTR szServiceName, LPCWSTR szDisplayName) {
    SC_HANDLE hService = NULL;
    BOOL bResult = TRUE;

    hService = OpenServiceW(hSCM, szServiceName, SERVICE_START | SERVICE_QUERY_STATUS);
    if (!hService) {
        wprintf(L"[-] OpenServiceW - %ls not found (error: %u)\n", szDisplayName, GetLastError());
        return FALSE;
    }

    SERVICE_STATUS_PROCESS ssStatus;
    DWORD dwBytesNeeded;
    if (QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus, sizeof(ssStatus), &dwBytesNeeded)) {
        if (ssStatus.dwCurrentState == SERVICE_RUNNING) {
            wprintf(L"[+] %ls is already running\n", szDisplayName);
            CloseServiceHandle(hService);
            return TRUE;
        }
    }

    if (!StartServiceW(hService, 0, NULL)) {
        DWORD dwErr = GetLastError();
        if (dwErr == ERROR_SERVICE_ALREADY_RUNNING) {
            wprintf(L"[+] %ls is already running\n", szDisplayName);
        }
        else {
            wprintf(L"[-] StartServiceW - %ls failed (error: %u)\n", szDisplayName, dwErr);
            bResult = FALSE;
        }
    }
    else {
        wprintf(L"[+] %ls started successfully\n", szDisplayName);
    }

    CloseServiceHandle(hService);
    return bResult;
}

int main() {
    SC_HANDLE hSCM = NULL;
    DWORD dwCount = 0;
    DWORD dwSuccess = 0;

    wprintf(L"[i] Windows Defender Re-Enabler\n");
    wprintf(L"[i] Attempting to start all Defender services...\n\n");

    hSCM = OpenSCManagerW(NULL, SERVICES_ACTIVE_DATABASEW, SC_MANAGER_CONNECT);
    if (!hSCM) {
        wprintf(L"[-] OpenSCManagerW failed (error: %u). Run as Administrator.\n", GetLastError());
        return 1;
    }

    dwCount = sizeof(g_Services) / sizeof(g_Services[0]);
    for (DWORD i = 0; i < dwCount; i++) {
        if (StartDefenderService(hSCM, g_Services[i].szName, g_Services[i].szDisplay)) {
            dwSuccess++;
        }
    }

    CloseServiceHandle(hSCM);

    wprintf(L"\n[i] %u/%u services started\n", dwSuccess, dwCount);

    if (dwSuccess == dwCount) {
        wprintf(L"[+] Windows Defender has been fully re-enabled!\n");
    }
    else {
        wprintf(L"[i] Some services could not be started. Defender may still be partially disabled.\n");
        wprintf(L"[i] Try restarting the system to fully restore Windows Defender.\n");
    }

    wprintf(L"\nPress any key to exit...\n");
    getchar();
    return (dwSuccess == dwCount) ? 0 : 1;
}
