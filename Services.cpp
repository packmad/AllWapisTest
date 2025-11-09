#include "Services.h"
#include "Utils.h"

#pragma comment(lib, "Advapi32.lib")

const wchar_t* ServiceName = L"CmdService";

void WINAPI ServiceMain(DWORD argc, LPWSTR* argv);
void WINAPI ServiceCtrlHandler(DWORD ctrlCode);
void InstallService();
void UninstallService();

SERVICE_STATUS serviceStatus;
SERVICE_STATUS_HANDLE serviceStatusHandle;

void Services::TestCreateService() {
    // Test install
    InstallService();
    bool success = (GetLastError() == ERROR_SUCCESS);  // Check if created

    // Test uninstall (cleanup)
    UninstallService();

    PrintTestResult(L"CreateService", success, success ? L"Service CmdService installed/uninstalled (runs cmd.exe as SYSTEM)" : L"Install failed: " + std::to_wstring(GetLastError()));
}

// Implementations (as in PoC)
void WINAPI ServiceMain(DWORD argc, LPWSTR* argv) {
    serviceStatusHandle = RegisterServiceCtrlHandler(ServiceName, ServiceCtrlHandler);
    if (!serviceStatusHandle) return;

    serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    serviceStatus.dwCurrentState = SERVICE_START_PENDING;
    serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    SetServiceStatus(serviceStatusHandle, &serviceStatus);

    serviceStatus.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(serviceStatusHandle, &serviceStatus);

    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    CreateProcess(L"C:\\Windows\\System32\\cmd.exe", nullptr, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    while (serviceStatus.dwCurrentState == SERVICE_RUNNING) Sleep(1000);
    serviceStatus.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(serviceStatusHandle, &serviceStatus);
}

void WINAPI ServiceCtrlHandler(DWORD ctrlCode) {
    if (ctrlCode == SERVICE_CONTROL_STOP || ctrlCode == SERVICE_CONTROL_SHUTDOWN) {
        serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        SetServiceStatus(serviceStatusHandle, &serviceStatus);
        serviceStatus.dwCurrentState = SERVICE_STOPPED;
        SetServiceStatus(serviceStatusHandle, &serviceStatus);
    }
}

void InstallService() {
    SC_HANDLE schSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);
    if (!schSCManager) return;

    wchar_t path[MAX_PATH];
    GetModuleFileName(nullptr, path, MAX_PATH);

    SC_HANDLE schService = CreateService(schSCManager, ServiceName, ServiceName, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, path, nullptr, nullptr, nullptr, nullptr, nullptr);
    if (schService) CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}

void UninstallService() {
    SC_HANDLE schSCManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (!schSCManager) return;

    SC_HANDLE schService = OpenService(schSCManager, ServiceName, SERVICE_STOP | DELETE);
    if (!schService) {
        CloseServiceHandle(schSCManager);
        return;
    }

    SERVICE_STATUS status;
    ControlService(schService, SERVICE_CONTROL_STOP, &status);
    DeleteService(schService);
    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
}