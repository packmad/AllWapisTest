#pragma once
#include <windows.h>
#include <TlHelp32.h>
#include <WtsApi32.h>
#include <shellapi.h>
#include <Wbemidl.h>
#include <comutil.h>

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "Wtsapi32.lib")

namespace ProcessThreadManagement {
    void TestWMIProcessCreate();
    void TestWTSEnumerateProcesses();
    void TestTerminateProcess();
    void TestZwTerminateProcess();
    void TestShellExecute();
    void TestNtSuspendProcess();
    void TestCreateThread();
    void TestNtSetInformationThread();
    void TestSuspendResumeThreads();
    void TestCreateProcessW();
    void TestCreateProcessWithLogonW();
    void TestCreateProcessWithTokenW();
    void TestCreateProcessAsUser();
    void TestCreateProcessInternalW();
}