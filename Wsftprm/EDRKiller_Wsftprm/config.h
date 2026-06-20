/*
    Store all variables and settings used across the project
*/

#pragma once
#include <wchar.h>


// Define path for the driver file
#define g_VULNDRIVERPATH L"\\System32\\Drivers\\"   // Default directory for runtime-loaded kernel drivers

// Declare variables for the vulnerable driver
extern const wchar_t g_VULNDRIVERNAME[];
extern const wchar_t g_VULNDRIVERFILENAME[];
extern const wchar_t g_VULNDRIVERSYMLINK[];

// Define IOCTL code
#define IOCTL_CODE 0x22201C   // Vulnerable IOCTL code

// Define the sleep time
#define g_SLEEPTIME 1000      // Time to sleep inbetween EDR process enumerations loops
