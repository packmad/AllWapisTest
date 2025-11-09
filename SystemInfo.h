#pragma once
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <Lmcons.h> // For UNLEN

namespace SystemInfo {
    void TestGetComputerName();
    void TestGetUserName();
    void TestGetNativeSystemInfo();
    void TestGetCommandLine();
}