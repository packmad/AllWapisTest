#pragma once
// Define to suppress deprecated Winsock warnings
#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

namespace Networking {
    void TestInternetConnect();
    void TestInternetReadFile();
    void TestGetAdaptersInfo();
    void TestGetIpNetTable();
    void TestIcmpEchoRequest();
    void TestGetExtendedTcpTable();
    void TestWinHttpConnect();
    void TestSocketConnect();
    void TestGetAddrInfo();
    void TestGetHostName();
    void TestSocketListen();
    void TestPortScanning();
}