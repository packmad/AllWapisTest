#include "MainHeader.h"
#include <tchar.h>
#include <windows.h>
#include <iostream>
#include <rpcasync.h>

using namespace std;


int processes() {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    std::wstring cmd(L"notepad.exe");

    if (!CreateProcess(
        NULL,   // lpApplicationName
        &cmd[0], // lpCommandLine
        NULL,   // lpProcessAttributes
        NULL,   // lpThreadAttributes
        FALSE,  // bInheritHandles
        0,      // dwCreationFlags
        NULL,   // lpEnvironment
        NULL,   // lpCurrentDirectory
        &si,    // lpStartupInfo
        &pi     // lpProcessInformation
    ))
    {
        printf("CreateProcess failed (%d).\n", GetLastError());
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}
