#include "SystemInfo.h"
#include "Utils.h"
#include <iomanip>

// Implementation of System Information tests
void SystemInfo::TestGetComputerName() {
    std::vector<wchar_t> name(MAX_COMPUTERNAME_LENGTH + 1);
    DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
   
    if (GetComputerNameW(name.data(), &size)) {
        PrintTestResult(L"GetComputerName", true,
            std::wstring(L"Computer: ") + name.data());
    } else {
        PrintTestResult(L"GetComputerName", false,
            L"Error: " + std::to_wstring(GetLastError()));
    }
}
   
void SystemInfo::TestGetUserName() {
    std::vector<wchar_t> name(UNLEN + 1);
    DWORD size = UNLEN + 1;
   
    if (GetUserNameW(name.data(), &size)) {
        PrintTestResult(L"GetUserName", true,
            std::wstring(L"User: ") + name.data());
    } else {
        PrintTestResult(L"GetUserName", false,
            L"Error: " + std::to_wstring(GetLastError()));
    }
}
   
void SystemInfo::TestGetNativeSystemInfo() {
    SYSTEM_INFO si;
    GetNativeSystemInfo(&si);
   
    std::wstring details = L"Processors: " + std::to_wstring(si.dwNumberOfProcessors) +
                          L", Page Size: " + std::to_wstring(si.dwPageSize) + L" bytes";
   
    PrintTestResult(L"GetNativeSystemInfo", true, details);
}
   
void SystemInfo::TestGetCommandLine() {
    LPWSTR cmdLine = GetCommandLineW();
    if (cmdLine != nullptr) {
        PrintTestResult(L"GetCommandLine", true,
            std::wstring(L"CmdLine: ") + cmdLine);
    } else {
        PrintTestResult(L"GetCommandLine", false);
    }
}