// Include modular headers
#include "SystemInfo.h"
#include "AccessControl.h"
#include "Com.h"
#include "Device.h"
#include "Disk.h"
#include "FileIO.h"
#include "Memory.h"
#include "Process.h"
#include "Registry.h"
#include "Networking.h"
#include "Services.h"
#include "Synchronization.h"
#include "Utils.h"

#include <string>
#include <iostream>
#include <map>
#include <set>
#include <vector>

int main(int argc, char* argv[]) {
    SetConsoleColor(COLOR_WHITE);

    std::set<std::string> selectedCategories;
    bool runAll = true;
    bool showHelp = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            showHelp = true;
            break;
        } else if (arg == "--check" && i + 1 < argc) {
            std::string category = argv[++i];
            selectedCategories.insert(category);
            runAll = false;
        }
        else {
            SetConsoleColor(COLOR_RED);
            std::cout << "Invalid argument option: " << arg << std::endl;
            SetConsoleColor(COLOR_WHITE);
            std::cout << "Run " << argv[0] << " --help for more info" << std::endl;
            return 0;
        }
    }

    if (showHelp) {
        std::wcout << L"Usage: " << argv[0] << L" [--check <type>]... [--help]\n";
        std::wcout << L"\nOptions:\n";
        std::wcout << L"  --check <type>      Enable specific check(s). Can be used multiple times. Valid types are:\n";
        std::wcout << L"    systeminfo        - SYSTEM INFORMATION APIs\n";
        std::wcout << L"    accesscontrol     - ACCESS CONTROL & PRIVILEGES APIs\n";
        std::wcout << L"    com               - COMPONENT OBJECT MODEL (COM) APIs\n";
        std::wcout << L"    devicemgmt        - DEVICE MANAGEMENT APIs\n";
        std::wcout << L"    diskmgmt          - DISK MANAGEMENT APIs\n";
        std::wcout << L"    fileio            - FILE I/O APIs\n";
        std::wcout << L"    memory            - MEMORY MANAGEMENT APIs\n";
        std::wcout << L"    process           - PROCESS & THREAD MANAGEMENT APIs\n";
        std::wcout << L"    registry          - REGISTRY APIs\n";
        std::wcout << L"    networking        - NETWORKING APIs\n";
        std::wcout << L"    services          - SERVICE MANAGEMENT APIs\n";
        std::wcout << L"    sync              - SYNCHRONIZATION APIs (Mutex)\n";
        std::wcout << L"\nNo --check argument runs all tests.\n";
        std::wcout << L"Run as Admin for privileged APIs.\n";
        return 0;
    }

    // Validate categories if specified
    if (!runAll) {
        std::map<std::string, bool> validCategories = {
            {"systeminfo", true},
            {"accesscontrol", true},
            {"com", true},
            {"devicemgmt", true},
            {"diskmgmt", true},
            {"fileio", true},
            {"memory", true},
            {"process", true},
            {"registry", true},
            {"networking", true},
            {"services", true},
            {"sync", true}
        };
        for (const auto& cat : selectedCategories) {
            if (validCategories.find(cat) == validCategories.end()) {
                SetConsoleColor(COLOR_RED);
                std::cerr << "Invalid category: " << cat << ". Use --help for list.\n";
                SetConsoleColor(COLOR_WHITE);
                return 1;
            }
        }
    }

    try {
        // System Information Tests
        if (runAll || selectedCategories.count("systeminfo")) {
            PrintSectionHeader(L"SYSTEM INFORMATION APIs");
            SystemInfo::TestGetComputerName();
            SystemInfo::TestGetUserName();
            SystemInfo::TestGetNativeSystemInfo();
            SystemInfo::TestGetCommandLine();
        }
       
        // Access Control Tests
        if (runAll || selectedCategories.count("accesscontrol")) {
            PrintSectionHeader(L"ACCESS CONTROL & PRIVILEGES APIs");
            AccessControl::TestAdjustTokenPrivileges();
            //AccessControl::TestNtAdjustPrivilegesToken();
            AccessControl::TestRtlAdjustPrivilege();
        }
       
        // COM Tests
        if (runAll || selectedCategories.count("com")) {
            PrintSectionHeader(L"COMPONENT OBJECT MODEL (COM) APIs");
            Com::TestITaskService();
        }
       
        // Device Management Tests
        if (runAll || selectedCategories.count("devicemgmt")) {
            PrintSectionHeader(L"DEVICE MANAGEMENT APIs");
            DeviceManagement::TestIOCTL_DISK_GET_DRIVE_GEOMETRY_EX();
            DeviceManagement::TestIOCTL_DISK_GET_LENGTH_INFO();
            DeviceManagement::TestIOCTL_DISK_IS_WRITABLE();
            DeviceManagement::TestIOCTL_STORAGE_GET_DEVICE_NUMBER();
        }
       
        // Disk Management Tests
        if (runAll || selectedCategories.count("diskmgmt")) {
            PrintSectionHeader(L"DISK MANAGEMENT APIs");
            DiskManagement::TestGetDriveType();
            DiskManagement::TestGetLogicalDrives();
        }
       
        // File I/O Tests
        if (runAll || selectedCategories.count("fileio")) {
            PrintSectionHeader(L"FILE I/O APIs");
            FileIO::TestCreateFileBasic();
            FileIO::TestCreateFileReadWrite();
            FileIO::TestCreateFileOverlapped();
        }
       
        // Memory Management Tests
        if (runAll || selectedCategories.count("memory")) {
            PrintSectionHeader(L"MEMORY MANAGEMENT APIs");
            Memory::TestVirtualAlloc();
            Memory::TestVirtualAllocEx();
        }
       
        // Process & Thread Management Tests
        if (runAll || selectedCategories.count("process")) {
            PrintSectionHeader(L"PROCESS & THREAD MANAGEMENT APIs");
            ProcessThreadManagement::TestWMIProcessCreate();
            ProcessThreadManagement::TestWTSEnumerateProcesses();
            ProcessThreadManagement::TestShellExecute();
            ProcessThreadManagement::TestNtSuspendProcess();
            ProcessThreadManagement::TestCreateThread();
            ProcessThreadManagement::TestNtSetInformationThread();
            ProcessThreadManagement::TestSuspendResumeThreads();
            ProcessThreadManagement::TestCreateProcessW();
            ProcessThreadManagement::TestCreateProcessWithLogonW();
            ProcessThreadManagement::TestCreateProcessWithTokenW();
            ProcessThreadManagement::TestCreateProcessAsUser();
            ProcessThreadManagement::TestCreateProcessInternalW();
            ProcessThreadManagement::TestTerminateProcess();
            ProcessThreadManagement::TestZwTerminateProcess();
        }
       
        // Registry Tests
        if (runAll || selectedCategories.count("registry")) {
            PrintSectionHeader(L"REGISTRY APIs");
            Registry::TestRegistryOperations();
        }

        // Networking Tests
        if (runAll || selectedCategories.count("networking")) {
            PrintSectionHeader(L"NETWORKING APIs");
            Networking::TestInternetConnect();
            Networking::TestInternetReadFile();
            Networking::TestGetAdaptersInfo();
            Networking::TestGetIpNetTable();
            Networking::TestIcmpEchoRequest();
            Networking::TestGetExtendedTcpTable();
            Networking::TestWinHttpConnect();
            Networking::TestSocketConnect();
            Networking::TestGetAddrInfo();
            Networking::TestGetHostName();
            Networking::TestSocketListen();
            Networking::TestPortScanning();
        }

        // Service Management Tests
        if (runAll || selectedCategories.count("services")) {
            PrintSectionHeader(L"SERVICE MANAGEMENT APIs");
            Services::TestCreateService();
        }

        // New: Synchronization (Mutex) Tests
        if (runAll || selectedCategories.count("sync")) {
            PrintSectionHeader(L"SYNCHRONIZATION APIs (Mutex)");
            Synchronization::TestCreateMutex();
        }
       
    } catch (const std::exception& e) {
        SetConsoleColor(COLOR_RED);
        std::cerr << "Exception occurred: " << e.what() << std::endl;
        SetConsoleColor(COLOR_WHITE);
    }
   
    return 0;
}