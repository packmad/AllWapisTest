#include "MainHeader.h"
#include <iostream>
#include <fstream>
#include <sstream>  
#include <string>
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>
using namespace std;


std::string filename = "C:\\Users\\dudee\\Desktop\\mormanti.txt";
LPCWSTR wfilename = L"C:\\Users\\dudee\\Desktop\\mormanti.txt";


void read() {
	std::fstream filestream(filename, std::ios::in);
	std::string shader;

	std::stringstream buffer;
	buffer << filestream.rdbuf();
	shader = buffer.str();

	std::cout << shader;
}

void write() {
    HANDLE hFile;
    char DataBuffer[] = "This is some test data to write to the file.";
    DWORD dwBytesToWrite = (DWORD)strlen(DataBuffer);
    DWORD dwBytesWritten = 0;
    BOOL bErrorFlag = FALSE;



    hFile = CreateFile(wfilename,                // name of the write
        GENERIC_WRITE,          // open for writing
        0,                      // do not share
        NULL,                   // default security
        CREATE_NEW,             // create new file only
        FILE_ATTRIBUTE_NORMAL,  // normal file
        NULL);                  // no attr. template

    if (hFile == INVALID_HANDLE_VALUE)
    {
        _tprintf(TEXT("Terminal failure: Unable to open file \"%s\" for write.\n"), wfilename);
        return;
    }

    _tprintf(TEXT("Writing %d bytes to %s.\n"), dwBytesToWrite, wfilename);

    bErrorFlag = WriteFile(
        hFile,           // open file handle
        DataBuffer,      // start of data to write
        dwBytesToWrite,  // number of bytes to write
        &dwBytesWritten, // number of bytes that were written
        NULL);            // no overlapped structure
    CloseHandle(hFile);
}


int filesystem() {
    write();
	read();
    DeleteFile(wfilename);
	return 0;
}