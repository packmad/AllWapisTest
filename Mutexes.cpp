#include "MainHeader.h"

using namespace std;

int mutexes() {
	HANDLE ghMutex;
    LPCWSTR userAgent = L"Mormanti";
    
    ghMutex = CreateMutex(
        NULL,              // default security attributes
        FALSE,             // initially not owned
        userAgent);  

    if (ghMutex == NULL)
    {
        printf("CreateMutex error: %d\n", GetLastError());
        return 1;
    }
    else {
        puts("mutex created\n");
    }
    return 0;
}