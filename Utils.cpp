#include "Utils.h"

void SetConsoleColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

void PrintTestResult(const std::wstring& testName, bool success, const std::wstring& details) {
    SetConsoleColor(COLOR_WHITE);
    std::wcout << L"[";
   
    if (success) {
        SetConsoleColor(COLOR_GREEN);
        std::wcout << L"PASS";
    } else {
        SetConsoleColor(COLOR_RED);
        std::wcout << L"FAIL";
    }
   
    SetConsoleColor(COLOR_WHITE);
    std::wcout << L"] " << testName;
   
    if (!details.empty()) {
        SetConsoleColor(COLOR_YELLOW);
        std::wcout << L" - " << details;
    }
   
    SetConsoleColor(COLOR_WHITE);
    std::wcout << std::endl;
}

void PrintSectionHeader(const std::wstring& sectionName) {
    SetConsoleColor(COLOR_YELLOW);
    std::wcout << L"\n" << std::wstring(80, L'=') << std::endl;
    std::wcout << L" " << sectionName << std::endl;
    std::wcout << std::wstring(80, L'=') << std::endl;
    SetConsoleColor(COLOR_WHITE);
}