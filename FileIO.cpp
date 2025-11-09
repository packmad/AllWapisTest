#include "FileIO.h"
#include "Utils.h"

// Implementation of File I/O tests
void FileIO::TestCreateFileBasic() {
    // Create temp file
    HANDLE hFile = CreateFile(L"C:\\temp\\test_basic.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    bool success = (hFile != INVALID_HANDLE_VALUE);
    if (success) {
        CloseHandle(hFile);
        DeleteFile(L"C:\\temp\\test_basic.txt");
        PrintTestResult(L"CreateFile Basic", true, L"Temp file created and deleted");
    } else {
        PrintTestResult(L"CreateFile Basic", false, L"Failed: " + std::to_wstring(GetLastError()));
    }
}

void FileIO::TestCreateFileReadWrite() {
    // Write and read back
    HANDLE hWrite = CreateFile(L"C:\\temp\\test_rw.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    bool success = (hWrite != INVALID_HANDLE_VALUE);
    if (success) {
        DWORD written;
        WriteFile(hWrite, "Test data", 9, &written, NULL);
        CloseHandle(hWrite);

        HANDLE hRead = CreateFile(L"C:\\temp\\test_rw.txt", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hRead != INVALID_HANDLE_VALUE) {
            char buf[10];
            DWORD read;
            ReadFile(hRead, buf, 9, &read, NULL);
            buf[9] = '\0';
            success = (read == 9 && strcmp(buf, "Test data") == 0);
            CloseHandle(hRead);
        }
        DeleteFile(L"C:\\temp\\test_rw.txt");
        PrintTestResult(L"CreateFile Read/Write", success, success ? L"Data written and read successfully" : L"Read mismatch");
    } else {
        PrintTestResult(L"CreateFile Read/Write", false, L"Create failed");
    }
}

void FileIO::TestCreateFileOverlapped() {
    // Basic overlapped I/O (async write)
    HANDLE hFile = CreateFile(L"C:\\temp\\test_overlapped.txt", GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);
    bool success = (hFile != INVALID_HANDLE_VALUE);
    if (success) {
        OVERLAPPED ol = { 0 };
        ol.OffsetHigh = 0;
        ol.Offset = 0;
        DWORD written;
        BOOL res = WriteFile(hFile, "Overlapped test", 15, &written, &ol);
        success = (res && written == 15);
        CloseHandle(hFile);
        DeleteFile(L"C:\\temp\\test_overlapped.txt");
        PrintTestResult(L"CreateFile Overlapped", success, success ? L"Async write completed" : L"Write failed: " + std::to_wstring(GetLastError()));
    } else {
        PrintTestResult(L"CreateFile Overlapped", false, L"Create failed");
    }
}