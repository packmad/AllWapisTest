#pragma once

#include <tchar.h>
#include <windows.h>
#include <winnt.h>
#include <iostream>
#include <cstring>
#include <iostream>
#include <windns.h> 
#include <stdio.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <sstream> 

extern std::string GetLastErrorAsString(LPCWSTR lpszFunction);
extern int processes(); 
extern int network(); 
extern int registry(); 
extern int mutexes(); 
extern int filesystem();
extern int service();