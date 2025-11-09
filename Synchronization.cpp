#include "Synchronization.h"
#include "Utils.h"

int InitMutex() {
    HANDLE ghMutex;
    LPCWSTR mutexName = L"Mormanti";
   
    ghMutex = CreateMutex(NULL, FALSE, mutexName);
    if (ghMutex == NULL) {
        printf("InitMutex error: %d\n", GetLastError());
        return 1;
    }
    return 0;
}

void Synchronization::TestCreateMutex() {
    int mutexResult = InitMutex();
    bool success = (mutexResult == 0);  // 0 means success (mutex created)

    // Verify handle and release
    HANDLE hMutex = CreateMutex(NULL, FALSE, L"Mormanti");
    bool handleValid = (hMutex != NULL);
    if (handleValid) {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
    }

    PrintTestResult(L"CreateMutex (Mormanti)", success && handleValid, success ? L"Mutex created and released" : L"Failed: " + std::to_wstring(GetLastError()));
}