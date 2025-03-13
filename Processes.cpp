#include "MainHeader.h"
#include <tchar.h>
#include <windows.h>
#include <iostream>
#include <rpcasync.h>
#include <initguid.h>

DEFINE_GUID(clsid, 0x13709620, 0xc279, 0x11ce, 0xa4, 0x9e, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00);

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

using namespace std;

BOOL CreateProcessWithCOM()
{
	printf("Creating process using the COM interface...\n");
	HRESULT hr;
	LPOLESTR clsidstr = nullptr;
	hr = ::StringFromCLSID(clsid, &clsidstr);
	printf("CLSID is %ls\n", clsidstr);
	hr = ::CoInitialize(nullptr);
	auto DllGetClassObject = (HRESULT(*)(REFCLSID, REFIID, LPVOID*))::GetProcAddress(::LoadLibraryW(L"shell32.dll"), "DllGetClassObject");
	if (!DllGetClassObject) {
		fprintf(stderr, "GetProcAddress with Error 0x%X", ::GetLastError());
		ExitProcess(EXIT_FAILURE);
		return FALSE;
	}
	IClassFactory* icf = nullptr;
	hr = DllGetClassObject(clsid, IID_IClassFactory, (void**)&icf);
	if (hr != S_OK) {
		printf("DllGetClassObject failed to do something. Error %d HRESULT 0x%08x\n", GetLastError(), (unsigned int)hr);
		::CoUninitialize();
		::ExitProcess(EXIT_FAILURE);
		return FALSE;
	}
	HMODULE shell32address = GetModuleHandleW(L"shell32.dll");
	IDispatch* id = nullptr;
	hr = icf->CreateInstance(nullptr, IID_IDispatch, (void**)&id);
	if (hr != S_OK) {
		printf("CreateInstance failed to do something. Error %d HRESULT 0x%08x\n", GetLastError(), (unsigned int)hr);
		::CoUninitialize();
		::ExitProcess(0);
		return FALSE;
	}
	const wchar_t* member = L"ShellExecute";
	DISPID dispid = 0;
	hr = id->GetIDsOfNames(IID_NULL, (LPOLESTR*)&member, 1, LOCALE_USER_DEFAULT, &dispid);
	if (hr != S_OK) {
		printf("GetIDsOfNames failed to do something. Error %d HRESULT 0x%08x\n", GetLastError(), (unsigned int)hr);

		CoUninitialize();
		ExitProcess(0);
		return FALSE;
	}
	// initialize parameters

	 //VARIANT describes arguments passed within DISPPARAMS.
	VARIANT args = { VT_EMPTY };
	args.vt = VT_BSTR;
	args.bstrVal = SysAllocString(L"cmd.exe");

	// Contains the arguments passed to ShellExecute method.
	DISPPARAMS dp = { &args, NULL, 1, 0 };
	VARIANT output = { VT_EMPTY };
	hr = id->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &dp, &output, NULL, NULL);
	if (hr != S_OK) {
		printf("Invoke failed to do something. Error %d HRESULT 0x%08x\n", GetLastError(), (unsigned int)hr);
		CoUninitialize();
		ExitProcess(0);
		return FALSE;
	}

	id->Release();
	icf->Release();
	SysFreeString(args.bstrVal);
	CoUninitialize();
	return TRUE;
}

BOOL CreateProcessWithPsAPI() 
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    std::wstring cmd(L"notepad.exe");
	
	printf("Creating process using the process API...\n");

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
		return FALSE;
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return TRUE;
}

int processes()
{
	CreateProcessWithPsAPI();
	CreateProcessWithCOM();
	return 0;
}