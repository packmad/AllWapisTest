#pragma once
#include <windows.h>
#include <iostream>
#include <winternl.h>
#include <vector>
#include <ntstatus.h>

namespace AccessControl {
    void TestAdjustTokenPrivileges();
    void TestNtAdjustPrivilegesToken();
    void TestRtlAdjustPrivilege();
}