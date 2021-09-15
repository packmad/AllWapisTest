#include "MainHeader.h"
#include <windows.h>
#include <iostream>
#include <cstring>
#include <iostream>
#include <tchar.h>



std::string GetLastErrorAsString(LPCWSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code

	LPSTR messageBuffer = nullptr;
	DWORD errorMessageID = GetLastError();
	if (errorMessageID == 0) {
		return std::string(); //No error message has been recorded
	};
	size_t size = FormatMessage(
		//FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		0x00000100 | 0x00001000 | 0x00000200,
		NULL,
		errorMessageID,
		//MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		0x10000000000,
		(LPTSTR)&messageBuffer,
		0, NULL);
	std::string message(messageBuffer, size);
	LocalFree(messageBuffer);
	return message;
}

