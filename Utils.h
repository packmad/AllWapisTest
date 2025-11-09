#pragma once
#include <windows.h>
#include <iostream>
#include <string>
#include <iomanip>

#pragma warning(disable: 28020)
#pragma warning(disable: 6031)
#pragma warning(disable: 4005)

#define COLOR_GREEN  10
#define COLOR_RED    12
#define COLOR_YELLOW 14
#define COLOR_WHITE  15

void SetConsoleColor(int color);
void PrintTestResult(const std::wstring& testName, bool success, const std::wstring& details = L"");
void PrintSectionHeader(const std::wstring& sectionName);