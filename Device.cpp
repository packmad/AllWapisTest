#include "Device.h"
#include "Utils.h"

// Implementation of Device Management tests
void DeviceManagement::TestIOCTL_DISK_GET_DRIVE_GEOMETRY_EX() {
    HANDLE hDevice = CreateFile(L"\\\\.\\PhysicalDrive0", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) {
        PrintTestResult(L"IOCTL_DISK_GET_DRIVE_GEOMETRY_EX", false, L"CreateFile failed (Admin required?): " + std::to_wstring(GetLastError()));
        return;
    }
    DISK_GEOMETRY_EX dg = { 0 };
    DWORD bytes;
    bool success = DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0, &dg, sizeof(dg), &bytes, NULL);
    CloseHandle(hDevice);
    PrintTestResult(L"IOCTL_DISK_GET_DRIVE_GEOMETRY_EX", success, success ? L"Disk size: " + std::to_wstring(dg.DiskSize.QuadPart) + L" bytes" : L"Failed: " + std::to_wstring(GetLastError()));
}

void DeviceManagement::TestIOCTL_DISK_GET_LENGTH_INFO() {
    HANDLE hDevice = CreateFile(L"\\\\.\\PhysicalDrive0", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) {
        PrintTestResult(L"IOCTL_DISK_GET_LENGTH_INFO", false, L"CreateFile failed");
        return;
    }
    GET_LENGTH_INFORMATION len = { 0 };
    DWORD bytes;
    bool success = DeviceIoControl(hDevice, IOCTL_DISK_GET_LENGTH_INFO, NULL, 0, &len, sizeof(len), &bytes, NULL);
    CloseHandle(hDevice);
    PrintTestResult(L"IOCTL_DISK_GET_LENGTH_INFO", success, success ? L"Disk length: " + std::to_wstring(len.Length.QuadPart) + L" bytes" : L"Failed");
}

void DeviceManagement::TestIOCTL_DISK_IS_WRITABLE() {
    HANDLE hDevice = CreateFile(L"\\\\.\\PhysicalDrive0", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) {
        PrintTestResult(L"IOCTL_DISK_IS_WRITABLE", false, L"CreateFile failed");
        return;
    }
    DWORD bytes;
    bool success = DeviceIoControl(hDevice, IOCTL_DISK_IS_WRITABLE, NULL, 0, NULL, 0, &bytes, NULL);
    CloseHandle(hDevice);
    DWORD err = GetLastError();
    std::wstring details = success ? L"Writable" : (err == ERROR_WRITE_PROTECT ? L"Write-protected" : L"Failed: " + std::to_wstring(err));
    PrintTestResult(L"IOCTL_DISK_IS_WRITABLE", success, details);
}

void DeviceManagement::TestIOCTL_STORAGE_GET_DEVICE_NUMBER() {
    HANDLE hDevice = CreateFile(L"\\\\.\\C:", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) {
        PrintTestResult(L"IOCTL_STORAGE_GET_DEVICE_NUMBER", false, L"CreateFile failed");
        return;
    }
    STORAGE_DEVICE_NUMBER sdn = { 0 };
    DWORD bytes;
    bool success = DeviceIoControl(hDevice, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &sdn, sizeof(sdn), &bytes, NULL);
    CloseHandle(hDevice);
    PrintTestResult(L"IOCTL_STORAGE_GET_DEVICE_NUMBER", success, success ? L"DeviceType: " + std::to_wstring(sdn.DeviceType) + L", Partition: " + std::to_wstring(sdn.PartitionNumber) : L"Failed");
}