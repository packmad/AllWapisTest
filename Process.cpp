#include "Process.h"
#include "Utils.h"
#include <winternl.h>
#include <ntstatus.h>
#include <tlhelp32.h>

// Template for safe COM release
template<typename T>
void SafeRelease(T** ppT) {
    if (*ppT) {
        (*ppT)->Release();
        *ppT = nullptr;
    }
}

// Implementation of Process & Thread Management tests
void ProcessThreadManagement::TestWMIProcessCreate() {
    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        PrintTestResult(L"WMIProcessCreate", false, L"CoInitializeEx failed");
        return;
    }

    hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
    if (FAILED(hr)) {
        CoUninitialize();
        PrintTestResult(L"WMIProcessCreate", false, L"CoInitializeSecurity failed");
        return;
    }

    IWbemLocator* pLoc = NULL;
    hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc);
    if (FAILED(hr)) {
        CoUninitialize();
        PrintTestResult(L"WMIProcessCreate", false, L"CoCreateInstance failed");
        return;
    }

    IWbemServices* pSvc = NULL;
    hr = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);
    bool success = SUCCEEDED(hr);
    if (success) {
        IWbemClassObject* pClass = NULL;
        hr = pSvc->GetObject(_bstr_t(L"Win32_Process"), 0, NULL, &pClass, NULL);
        if (SUCCEEDED(hr)) {
            IWbemClassObject* pIn = NULL;
            hr = pClass->GetMethod(L"Create", 0, &pIn, NULL);
            if (SUCCEEDED(hr)) {
                IWbemClassObject* pInInst = NULL;
                hr = pIn->SpawnInstance(0, &pInInst);
                if (SUCCEEDED(hr)) {
                    VARIANT var; 
                    VariantInit(&var); 
                    var.vt = VT_BSTR; 
                    var.bstrVal = SysAllocString(L"notepad.exe");
                    hr = pInInst->Put(L"CommandLine", 0, &var, 0);
                    VariantClear(&var);
                    if (SUCCEEDED(hr)) {
                        IWbemClassObject* pOut = NULL;
                        hr = pSvc->ExecMethod(_bstr_t(L"Win32_Process"), _bstr_t(L"Create"), 0, NULL, pInInst, &pOut, NULL);
                        success = SUCCEEDED(hr);
                        if (pOut) SafeRelease(&pOut);
                    }
                    SafeRelease(&pInInst);
                }
                SafeRelease(&pIn);
            }
            SafeRelease(&pClass);
        }
    }
    if (pSvc) SafeRelease(&pSvc);
    if (pLoc) SafeRelease(&pLoc);
    CoUninitialize();
    PrintTestResult(L"WMIProcessCreate", success, success ? L"Process created via WMI" : L"ExecMethod failed: 0x" + std::to_wstring(hr));
}

void ProcessThreadManagement::TestWTSEnumerateProcesses() {
    PWTS_PROCESS_INFO info = NULL;
    DWORD count = 0;
    BOOL res = WTSEnumerateProcesses(WTS_CURRENT_SERVER_HANDLE, 0, 1, &info, &count);
    bool success = (res && count > 0);
    if (success) {
        WTSFreeMemory(info);
    }
    PrintTestResult(L"WTSEnumerateProcesses", success, success ? L"Enumerated " + std::to_wstring(count) + L" processes" : L"Failed: " + std::to_wstring(GetLastError()));
}

void ProcessThreadManagement::TestTerminateProcess() {
    // Target: notepad.exe if running
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    bool found = false;
    if (snap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe = { sizeof(pe) };
        if (Process32FirstW(snap, &pe)) {
            do {
                if (_wcsicmp(pe.szExeFile, L"notepad.exe") == 0) {
                    HANDLE hProc = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                    if (hProc) {
                        BOOL term = TerminateProcess(hProc, 0);
                        CloseHandle(hProc);
                        found = true;
                        PrintTestResult(L"TerminateProcess", term, term ? L"notepad.exe terminated" : L"Terminate failed");
                        break;
                    }
                }
            } while (Process32NextW(snap, &pe));
        }
        CloseHandle(snap);
    }
    if (!found) PrintTestResult(L"TerminateProcess", false, L"notepad.exe not found (start one?)");
}

void ProcessThreadManagement::TestZwTerminateProcess() {
    typedef NTSTATUS(NTAPI* ZwTerminateProcessFn)(HANDLE, NTSTATUS);
    HMODULE hNtdll = GetModuleHandle(L"ntdll.dll");
    ZwTerminateProcessFn pZw = (ZwTerminateProcessFn)GetProcAddress(hNtdll, "ZwTerminateProcess");
    if (!pZw) {
        PrintTestResult(L"ZwTerminateProcess", false, L"GetProcAddress failed");
        return;
    }

    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    DWORD pid = 0;
    if (snap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe = { sizeof(pe) };
        if (Process32FirstW(snap, &pe)) {
            do {
                if (_wcsicmp(pe.szExeFile, L"notepad.exe") == 0) {
                    pid = pe.th32ProcessID;
                    break;
                }
            } while (Process32NextW(snap, &pe));
        }
        CloseHandle(snap);
    }
    
    if (pid == 0) {
        PrintTestResult(L"ZwTerminateProcess", false, L"notepad.exe not found");
        return;
    }
    
    HANDLE hProc = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (!hProc) {
        PrintTestResult(L"ZwTerminateProcess", false, L"OpenProcess failed");
        return;
    }
    NTSTATUS status = pZw(hProc, 0);
    CloseHandle(hProc);
    PrintTestResult(L"ZwTerminateProcess", status == 0, status == 0 ? L"Process terminated" : L"Failed: " + std::to_wstring(status));
}

void ProcessThreadManagement::TestShellExecute() {
    HINSTANCE res = ShellExecute(NULL, L"open", L"cmd.exe", L"/C echo ShellExecute Test > C:\\temp\\shell.txt", NULL, SW_HIDE);
    bool success = ((intptr_t)res > 32);
    PrintTestResult(L"ShellExecute", success, success ? L"Command executed" : L"Failed: " + std::to_wstring((intptr_t)res));
}

void ProcessThreadManagement::TestNtSuspendProcess() {
    typedef LONG(NTAPI* NtSuspendProcess)(HANDLE);
    HMODULE hNtdll = GetModuleHandle(L"ntdll.dll");
    NtSuspendProcess pNt = (NtSuspendProcess)GetProcAddress(hNtdll, "NtSuspendProcess");
    if (!pNt) {
        PrintTestResult(L"NtSuspendProcess", false, L"GetProcAddress failed"); 
        return;
    }

    // Target notepad
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    DWORD pid = 0;
    if (snap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe = { sizeof(pe) };
        if (Process32FirstW(snap, &pe)) {
            do {
                if (_wcsicmp(pe.szExeFile, L"notepad.exe") == 0) {
                    pid = pe.th32ProcessID;
                    break;
                }
            } while (Process32NextW(snap, &pe));
        }
        CloseHandle(snap);
    }
    if (pid == 0) {
        PrintTestResult(L"NtSuspendProcess", false, L"notepad.exe not found");
        return;
    }

    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProc) {
        PrintTestResult(L"NtSuspendProcess", false, L"OpenProcess failed");
        return;
    }
    LONG status = pNt(hProc);
    
    // Resume after test
    typedef LONG(NTAPI* NtResumeProcess)(HANDLE);
    NtResumeProcess pResume = (NtResumeProcess)GetProcAddress(hNtdll, "NtResumeProcess");
    if (pResume) {
        pResume(hProc);  // Use same handle
    }
    CloseHandle(hProc);
    
    PrintTestResult(L"NtSuspendProcess", status == 0, status == 0 ? L"Process suspended/resumed" : L"Failed: " + std::to_wstring(status));
}

void ProcessThreadManagement::TestCreateThread() {
    
    struct ThreadData {
        static DWORD WINAPI ThreadFunc1(LPVOID lpParam) {
            Sleep(1000);
            std::wcout << L"Thread 1 done\n";
            return 0;
        }
        
        static DWORD WINAPI ThreadFunc2(LPVOID lpParam) {
            for (int i = 0; i < 5; i++) {
                std::wcout << L"Thread 2: " << i << L"\n";
            }
            return 0;
        }
    };

    HANDLE hThread1 = CreateThread(NULL, 0, ThreadData::ThreadFunc1, NULL, 0, NULL);
    HANDLE hThread2 = CreateThread(NULL, 0, ThreadData::ThreadFunc2, NULL, 0, NULL);

    if (hThread1 && hThread2) {
        WaitForSingleObject(hThread1, INFINITE);
        WaitForSingleObject(hThread2, INFINITE);
        CloseHandle(hThread1);
        CloseHandle(hThread2);
        PrintTestResult(L"CreateThread", true, L"Threads created and waited");
    } else {
        if (hThread1) CloseHandle(hThread1);
        if (hThread2) CloseHandle(hThread2);
        PrintTestResult(L"CreateThread", false, L"CreateThread failed");
    }
}

void ProcessThreadManagement::TestNtSetInformationThread() {
    typedef NTSTATUS(NTAPI* pNtSetInformationThread)(HANDLE, ULONG, PVOID, ULONG);
    HMODULE hNtdll = GetModuleHandle(L"ntdll.dll");
    pNtSetInformationThread pNt = (pNtSetInformationThread)GetProcAddress(hNtdll, "NtSetInformationThread");
    if (!pNt) {
        PrintTestResult(L"NtSetInformationThread", false, L"GetProcAddress failed");
    }

    // Hide current thread
    NTSTATUS status = pNt(GetCurrentThread(), 0x11 /*ThreadHideFromDebugger*/, NULL, 0);
    PrintTestResult(L"NtSetInformationThread", status == 0, status == 0 ? L"Thread hidden from debugger" : L"Failed: " + std::to_wstring(status));
}

void ProcessThreadManagement::TestSuspendResumeThreads() {
    struct ThreadData {
        static DWORD WINAPI ThreadFunc(LPVOID lpParam) {
            Sleep(5000);  // Use finite time instead of INFINITE
            return 0;
        }
    };
    
    DWORD threadId;
    HANDLE hThread = CreateThread(NULL, 0, ThreadData::ThreadFunc, NULL, 0, &threadId);
    if (!hThread) {
        PrintTestResult(L"SuspendResumeThreads", false, L"CreateThread failed");
        return;
    }

    DWORD suspendCount = SuspendThread(hThread);
    bool suspendSuccess = (suspendCount != (DWORD)-1);
    
    Sleep(100);  // Give time for suspend to take effect
    
    DWORD resumeCount = ResumeThread(hThread);
    bool resumeSuccess = (resumeCount != (DWORD)-1);
    
    WaitForSingleObject(hThread, 1000);
    TerminateThread(hThread, 0);
    CloseHandle(hThread);
    
    PrintTestResult(L"SuspendResumeThreads", suspendSuccess && resumeSuccess, 
        L"Suspended (" + std::to_wstring(suspendCount) + L") and resumed (" + std::to_wstring(resumeCount) + L")");
}

void ProcessThreadManagement::TestCreateProcessW() {
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };
    WCHAR cmd[] = L"notepad.exe";
    bool success = CreateProcessW(NULL, cmd, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
    if (success) {
        Sleep(1000);
        TerminateProcess(pi.hProcess, 0);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        PrintTestResult(L"CreateProcessW", true, L"Notepad launched via CreateProcessW");
    } else {
        PrintTestResult(L"CreateProcessW", false, L"Failed: " + std::to_wstring(GetLastError()));
    }
}

void ProcessThreadManagement::TestCreateProcessWithLogonW() {
    PrintTestResult(L"CreateProcessWithLogonW", false, L"Requires valid credentials - test skipped");
}

void ProcessThreadManagement::TestCreateProcessWithTokenW() {
    HANDLE hToken = NULL;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_DUPLICATE | TOKEN_QUERY | TOKEN_ASSIGN_PRIMARY, &hToken)) {
        PrintTestResult(L"CreateProcessWithTokenW", false, L"OpenProcessToken failed: " + std::to_wstring(GetLastError()));
        return;
    }
    HANDLE hDupToken = NULL;
    if (!DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL, SecurityImpersonation, TokenPrimary, &hDupToken)) {
        CloseHandle(hToken);
        PrintTestResult(L"CreateProcessWithTokenW", false, L"DuplicateTokenEx failed: " + std::to_wstring(GetLastError()));
        return;
    }

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };
    WCHAR cmd[] = L"notepad.exe";
    bool success = CreateProcessWithTokenW(hDupToken, LOGON_WITH_PROFILE, NULL, cmd, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
    CloseHandle(hToken);
    CloseHandle(hDupToken);
    if (success) {
        Sleep(1000);
        TerminateProcess(pi.hProcess, 0);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        PrintTestResult(L"CreateProcessWithTokenW", true, L"Process launched with token");
    } else {
        PrintTestResult(L"CreateProcessWithTokenW", false, L"Failed: " + std::to_wstring(GetLastError()));
    }
}

void ProcessThreadManagement::TestCreateProcessAsUser() {
    HANDLE hToken = NULL;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_DUPLICATE | TOKEN_QUERY | TOKEN_ASSIGN_PRIMARY, &hToken)) {
        PrintTestResult(L"CreateProcessAsUser", false, L"OpenProcessToken failed: " + std::to_wstring(GetLastError()));
        return;
    }
    HANDLE hDupToken = NULL;
    if (!DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL, SecurityImpersonation, TokenPrimary, &hDupToken)) {
        CloseHandle(hToken);
        PrintTestResult(L"CreateProcessAsUser", false, L"DuplicateTokenEx failed: " + std::to_wstring(GetLastError()));
        return;
    }

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };
    WCHAR cmd[] = L"notepad.exe";
    bool success = CreateProcessAsUserW(hDupToken, NULL, cmd, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
    CloseHandle(hToken);
    CloseHandle(hDupToken);
    if (success) {
        Sleep(1000);
        TerminateProcess(pi.hProcess, 0);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        PrintTestResult(L"CreateProcessAsUser", true, L"Process launched as user");
    } else {
        PrintTestResult(L"CreateProcessAsUser", false, L"Failed: " + std::to_wstring(GetLastError()));
    }
}

void ProcessThreadManagement::TestCreateProcessInternalW() {
    typedef BOOL(WINAPI* pCreateProcessInternalW)(
        HANDLE hToken,
        LPCWSTR lpApplicationName,
        LPWSTR lpCommandLine,
        LPSECURITY_ATTRIBUTES lpProcessAttributes,
        LPSECURITY_ATTRIBUTES lpThreadAttributes,
        BOOL bInheritHandles,
        DWORD dwCreationFlags,
        LPVOID lpEnvironment,
        LPCWSTR lpCurrentDirectory,
        LPSTARTUPINFOW lpStartupInfo,
        LPPROCESS_INFORMATION lpProcessInformation,
        PHANDLE hNewToken);

    HMODULE hKernelBase = GetModuleHandle(L"kernelbase.dll");  // It's in kernelbase, not kernel32
    if (!hKernelBase) {
        PrintTestResult(L"CreateProcessInternalW", false, L"GetModuleHandle(kernelbase.dll) failed");
        return;
    }
    
    pCreateProcessInternalW pInternal = (pCreateProcessInternalW)GetProcAddress(hKernelBase, "CreateProcessInternalW");
    if (!pInternal) {
        PrintTestResult(L"CreateProcessInternalW", false, L"GetProcAddress failed");
        return;
    }

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };
    WCHAR cmd[] = L"notepad.exe";
    
    // Actually call the function
    bool success = pInternal(NULL, NULL, cmd, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi, NULL);
    
    if (success) {
        Sleep(1000);
        TerminateProcess(pi.hProcess, 0);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        PrintTestResult(L"CreateProcessInternalW", true, L"Process launched via internal API");
    } else {
        PrintTestResult(L"CreateProcessInternalW", false, L"Failed: " + std::to_wstring(GetLastError()));
    }
}