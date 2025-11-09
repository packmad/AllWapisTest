#include "AccessControl.h"
#include "Utils.h"

// Implementation of Access Control tests
void AccessControl::TestAdjustTokenPrivileges() {
    HANDLE hToken;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        PrintTestResult(L"AdjustTokenPrivileges", false, L"OpenProcessToken failed: " + std::to_wstring(GetLastError()));
        return;
    }
    auto SetPrivilege = [&](LPCWSTR lpszPrivilege, BOOL bEnablePrivilege) -> bool {
        TOKEN_PRIVILEGES tp;
        LUID luid;
        if (!LookupPrivilegeValue(NULL, lpszPrivilege, &luid)) return false;
        tp.PrivilegeCount = 1;
        tp.Privileges[0].Luid = luid;
        tp.Privileges[0].Attributes = bEnablePrivilege ? SE_PRIVILEGE_ENABLED : 0;
        if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) return false;
        if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) return false;
        return true;
        };

    std::vector<std::pair<LPCWSTR, std::wstring>> privileges = {
        {SE_DEBUG_NAME, L"SeDebugPrivilege"},
        {SE_TAKE_OWNERSHIP_NAME, L"SeTakeOwnershipPrivilege"},
        {SE_LOAD_DRIVER_NAME, L"SeLoadDriverPrivilege"}
    };

    std::wstring failed_privileges;
    bool all_success = true;

    for (const auto& priv : privileges) {
        if (!SetPrivilege(priv.first, TRUE)) {
            if (!failed_privileges.empty()) {
                failed_privileges += L", ";
            }
            failed_privileges += priv.second;
            all_success = false;
            std::wcout << L"Failed to enable " << priv.second << L".\n";
        }
        else {
            std::wcout << priv.second << L" enabled successfully.\n";
        }
    }

    CloseHandle(hToken);

    std::wstring message;
    if (all_success) {
        message = L"All privileges enabled successfully";
    }
    else {
        message = L"Failed to enable privileges: " + failed_privileges;
    }
    PrintTestResult(L"AdjustTokenPrivileges", all_success, message);
}

void AccessControl::TestNtAdjustPrivilegesToken() {
    typedef NTSTATUS(NTAPI* PFN_NtOpenProcessToken)(HANDLE, ACCESS_MASK, PHANDLE);
    typedef NTSTATUS(NTAPI* PFN_NtAdjustPrivilegesToken)(HANDLE, BOOLEAN, PTOKEN_PRIVILEGES, ULONG, PTOKEN_PRIVILEGES, PULONG);

    HMODULE hNtdll = GetModuleHandle(L"ntdll.dll");
    if (!hNtdll) {
        PrintTestResult(L"NtAdjustPrivilegesToken", false, L"Failed to load ntdll.dll");
        return;
    }

    PFN_NtOpenProcessToken pNtOpen = (PFN_NtOpenProcessToken)GetProcAddress(hNtdll, "NtOpenProcessToken");
    PFN_NtAdjustPrivilegesToken pNtAdjust = (PFN_NtAdjustPrivilegesToken)GetProcAddress(hNtdll, "NtAdjustPrivilegesToken");
    if (!pNtOpen || !pNtAdjust) {
        PrintTestResult(L"NtAdjustPrivilegesToken", false, L"Failed to get NT functions");
        return;
    }

    HANDLE token;
    NTSTATUS status = pNtOpen(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token);
    if (status != 0) {
        PrintTestResult(L"NtAdjustPrivilegesToken", false, L"NtOpenProcessToken failed: " + std::to_wstring(status));
        return;
    }

    std::vector<std::wstring> privileges = { SE_DEBUG_NAME, SE_TAKE_OWNERSHIP_NAME };
    TOKEN_PRIVILEGES tp = { static_cast<DWORD>(privileges.size()) };
    bool allSuccess = true;
    for (size_t i = 0; i < privileges.size(); ++i) {
        LUID luid;
        if (!LookupPrivilegeValue(NULL, privileges[i].c_str(), &luid)) {
            allSuccess = false;
            continue;
        }
        tp.Privileges[i].Luid = luid;
        tp.Privileges[i].Attributes = SE_PRIVILEGE_ENABLED;
    }

    status = pNtAdjust(token, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
    CloseHandle(token);
    PrintTestResult(L"NtAdjustPrivilegesToken", status == 0 && allSuccess, status == 0 ? L"Privileges enabled" : L"Failed: " + std::to_wstring(status));
}

void AccessControl::TestRtlAdjustPrivilege() {
    typedef NTSTATUS(NTAPI* pfnRtlAdjustPrivilege)(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN);
    HMODULE hNtdll = LoadLibraryW(L"ntdll.dll");
    if (!hNtdll) {
        PrintTestResult(L"RtlAdjustPrivilege", false);
        return;
    }
    pfnRtlAdjustPrivilege pRtl = (pfnRtlAdjustPrivilege)GetProcAddress(hNtdll, "RtlAdjustPrivilege");
    if (!pRtl) {
        FreeLibrary(hNtdll);
        PrintTestResult(L"RtlAdjustPrivilege", false);
        return;
    }

    #define SE_SHUTDOWN_PRIVILEGE 19
    #define SE_DEBUG_PRIVILEGE 20
    #define SE_SYSTEMTIME_PRIVILEGE 12

    BOOLEAN wasEnabled;
    bool success = (pRtl(SE_DEBUG_PRIVILEGE, TRUE, FALSE, &wasEnabled) == 0);
    FreeLibrary(hNtdll);
    PrintTestResult(L"RtlAdjustPrivilege", success, success ? L"SeDebugPrivilege enabled" : L"Failed to enable");
}