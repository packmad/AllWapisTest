#include "Registry.h"
#include "Utils.h"
#include <tchar.h>  

BOOL Is_RegKeyExists(HKEY hKey, const TCHAR* lpSubKey) {
    HKEY hkResult = NULL;
    if (RegOpenKeyEx(hKey, lpSubKey, NULL, KEY_READ, &hkResult) == ERROR_SUCCESS) {
        RegCloseKey(hkResult);
        return TRUE;
    }
    return FALSE;
}

bool CreateRegistryKey(HKEY hKeyRoot, LPCTSTR pszSubKey) {
    HKEY hKey;
    DWORD dwFunc;
    LONG lRet;
    SECURITY_DESCRIPTOR SD;
    SECURITY_ATTRIBUTES SA;
    if (!InitializeSecurityDescriptor(&SD, SECURITY_DESCRIPTOR_REVISION)) return false;
    if (!SetSecurityDescriptorDacl(&SD, true, NULL, false)) return false;
    SA.nLength = sizeof(SA);
    SA.lpSecurityDescriptor = &SD;
    SA.bInheritHandle = false;

    lRet = RegCreateKeyEx(hKeyRoot, pszSubKey, 0, (LPTSTR)NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, &SA, &hKey, &dwFunc);
    if (lRet == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return true;
    }
    SetLastError((DWORD)lRet);
    return false;
}

// Enhanced test incorporating provided registry() logic
void Registry::TestRegistryOperations() {
    // Test 1: Check if key exists (e.g., Run key)
    bool keyExists = Is_RegKeyExists(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"));
    PrintTestResult(L"Is_RegKeyExists (Run key)", keyExists, keyExists ? L"Run key exists" : L"Run key missing");

    // Test 2: Create key with security (Console\\Mormanti)
    std::wstring testKey = L"Console\\Mormanti";
    bool createSuccess = CreateRegistryKey(HKEY_CURRENT_USER, testKey.c_str());
    PrintTestResult(L"CreateRegistryKey (Console\\Mormanti)", createSuccess, createSuccess ? L"Key created with DACL" : L"Create failed: " + std::to_wstring(GetLastError()));

    // Test 3: Delete the created key (from registry() logic)
    LONG deleteResult = RegDeleteKey(HKEY_CURRENT_USER, testKey.c_str());
    bool deleteSuccess = (deleteResult == ERROR_SUCCESS);
    PrintTestResult(L"RegDeleteKey (Cleanup)", deleteSuccess, deleteSuccess ? L"Test key deleted" : L"Delete failed: " + std::to_wstring(deleteResult));
}