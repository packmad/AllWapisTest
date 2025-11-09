# AllWapisTest - API Testing Framework

This project generates a native PE file that triggers a good part of the Windows APIs, designed to validate and demonstrate various Windows system APIs across multiple categories.

## Building the Project

### Prerequisites
- Visual Studio 2019 or later
- Windows SDK

## Usage
**Run as Administrator**, some API tests require administrative privileges to function correctly.

### Run All Tests
```bash
AllWapisTest.exe
```

### Run Specific Category
```bash
AllWapisTest.exe --check systeminfo
```

### Run Multiple Categories
```bash
AllWapisTest.exe --check process --check networking --check memory
```

### Display Help
```bash
AllWapisTest.exe --help
```

## Current API Categories supported

### 1. System Information APIs (`systeminfo`)
- `GetComputerName` - Retrieve computer name
- `GetUserName` - Get current username
- `GetNativeSystemInfo` - System architecture information
- `GetCommandLine` - Command line arguments

### 2. Access Control & Privileges APIs (`accesscontrol`)
- `AdjustTokenPrivileges` - Modify process token privileges
- `RtlAdjustPrivilege` - Native privilege adjustment

### 3. Component Object Model (COM) APIs (`com`)
- `ITaskService` - Task Scheduler COM interface

### 4. Device Management APIs (`devicemgmt`)
- `IOCTL_DISK_GET_DRIVE_GEOMETRY_EX` - Drive geometry information
- `IOCTL_DISK_GET_LENGTH_INFO` - Disk length information
- `IOCTL_DISK_IS_WRITABLE` - Write capability check
- `IOCTL_STORAGE_GET_DEVICE_NUMBER` - Device number retrieval

### 5. Disk Management APIs (`diskmgmt`)
- `GetDriveType` - Identify drive types
- `GetLogicalDrives` - Enumerate logical drives

### 6. File I/O APIs (`fileio`)
- `CreateFile` (Basic) - File handle creation
- `CreateFile` (Read/Write) - File operations
- `CreateFile` (Overlapped) - Asynchronous I/O

### 7. Memory Management APIs (`memory`)
- `VirtualAlloc` - Memory allocation
- `VirtualAllocEx` - Remote process memory allocation

### 8. Process & Thread Management APIs (`process`)
- `WMI Process Create` - WMI-based process creation
- `WTSEnumerateProcesses` - Terminal Services process enumeration
- `ShellExecute` - Shell-based execution
- `NtSuspendProcess` - Native process suspension
- `CreateThread` - Thread creation
- `NtSetInformationThread` - Thread information modification
- `SuspendThread/ResumeThread` - Thread state control
- `CreateProcessW` - Standard process creation
- `CreateProcessWithLogonW` - Process creation with credentials
- `CreateProcessWithTokenW` - Token-based process creation
- `CreateProcessAsUser` - User context process creation
- `CreateProcessInternalW` - Internal process creation
- `TerminateProcess` - Process termination
- `ZwTerminateProcess` - Native process termination

### 9. Registry APIs (`registry`)
- Registry read/write operations
- Key creation and deletion

### 10. Networking APIs (`networking`)
- `InternetConnect` - WinINet connection
- `InternetReadFile` - HTTP data reading
- `GetAdaptersInfo` - Network adapter information
- `GetIpNetTable` - ARP table retrieval
- `IcmpSendEcho` - ICMP echo request
- `GetExtendedTcpTable` - TCP connection table
- `WinHttpConnect` - WinHTTP connection
- `socket/connect` - BSD socket operations
- `getaddrinfo` - DNS resolution
- `gethostname` - Local hostname
- `listen` - Socket listening
- Port scanning functionality

### 11. Service Management APIs (`services`)
- `CreateService` - Service creation and management

### 12. Synchronization APIs (`sync`)
- `CreateMutex` - Mutex creation and management

## Acknowledgments

- Inspired by [WindowsAP1 project](https://github.com/DebugPrivilege/WindowsAP1/tree/main)
- Windows API documentation by Microsoft

## Support

For issues, questions, or contributions, please open an issue on the project repository.