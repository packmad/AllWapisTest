#include "Disk.h"
#include "Utils.h"

void DiskManagement::TestGetDriveType() {
    wchar_t drive[] = L"A:\\";
    int validDrives = 0;
    std::wstring types;
    for (wchar_t letter = L'A'; letter <= L'Z'; ++letter) {
        drive[0] = letter;
        UINT type = GetDriveType(drive);
        if (type != DRIVE_NO_ROOT_DIR && type != DRIVE_UNKNOWN) {
            validDrives++;
            std::wstring typeStr;
            switch (type) {
            case DRIVE_FIXED: typeStr = L"Fixed"; break;
            case DRIVE_REMOVABLE: typeStr = L"Removable"; break;
            case DRIVE_REMOTE: typeStr = L"Remote"; break;
            case DRIVE_CDROM: typeStr = L"CDROM"; break;
            case DRIVE_RAMDISK: typeStr = L"RAMDisk"; break;
            }
            types += std::wstring(1, letter) + L": " + typeStr + L", ";
        }
    }
    PrintTestResult(L"GetDriveType", validDrives > 0, L"Found " + std::to_wstring(validDrives) + L" drives: " + types.substr(0, types.length() - 2));
}
   
void DiskManagement::TestGetLogicalDrives() {
    DWORD drives = GetLogicalDrives();
    if (drives != 0) {
        int count = 0;
        std::wstring driveList;
        for (int i = 0; i < 26; i++) {
            if (drives & (1 << i)) {
                count++;
                driveList += std::wstring(1, 'A' + i) + L": ";
            }
        }
        PrintTestResult(L"GetLogicalDrives", true, L"Found " + std::to_wstring(count) + L" drives: " + driveList);
    } else {
        PrintTestResult(L"GetLogicalDrives", false, L"Error: " + std::to_wstring(GetLastError()));
    }
}