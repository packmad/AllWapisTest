#include <iostream>
#include <string>
#include <exception>
#include <windows.h>
#include <tchar.h>
#include "MainHeader.h"

BOOL Is_RegKeyExists(HKEY hKey, const TCHAR* lpSubKey)
{
	HKEY hkResult = NULL;
	TCHAR lpData[1024] = { 0 };
	DWORD cbData = MAX_PATH;

	if (RegOpenKeyEx(hKey, lpSubKey, NULL, KEY_READ, &hkResult) == ERROR_SUCCESS)
	{
		RegCloseKey(hkResult);
		return TRUE;
	}

	return FALSE;
}

bool CreateRegistryKey(HKEY hKeyRoot, LPCTSTR pszSubKey)
{
	HKEY hKey;
	DWORD dwFunc;
	LONG  lRet;

	//------------------------------------------------------------------------------

	SECURITY_DESCRIPTOR SD;
	SECURITY_ATTRIBUTES SA;

	if (!InitializeSecurityDescriptor(&SD, SECURITY_DESCRIPTOR_REVISION))
		return false;

	if (!SetSecurityDescriptorDacl(&SD, true, 0, false))
		return false;

	SA.nLength = sizeof(SA);
	SA.lpSecurityDescriptor = &SD;
	SA.bInheritHandle = false;

	//------------------------------------------------------------------------------

	lRet = RegCreateKeyEx(
		hKeyRoot,
		pszSubKey,
		0,
		(LPTSTR)NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_WRITE,
		&SA,
		&hKey,
		&dwFunc
	);

	if (lRet == ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		hKey = (HKEY)NULL;
		return true;
	}

	SetLastError((DWORD)lRet);
	return false;
}

int registry() {
	bool rke = false;
	
	rke = Is_RegKeyExists(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"));

	HKEY hKey;
	PHKEY phKey = &hKey;
	std::wstring k(L"Console\\Mormanti");
	LPCWSTR kp = k.c_str();
	

	if (RegCreateKeyEx(HKEY_CURRENT_USER, kp, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, phKey, NULL) == ERROR_SUCCESS) {
		puts("Key Created!!!");
	}

	if (RegDeleteKey(HKEY_CURRENT_USER, kp) == ERROR_SUCCESS) {
		puts("Key Deleted!!!");
	}

	return 0;
}