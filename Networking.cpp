#include "Networking.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <wininet.h>
#include <winhttp.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#include <vector>
#include <fstream>

#include "Utils.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "iphlpapi.lib")

// Helper to convert IP to string
std::string IpToString(DWORD ip) {
    char buf[16];
    sprintf_s(buf, "%d.%d.%d.%d", (ip >> 0) & 0xFF, (ip >> 8) & 0xFF, (ip >> 16) & 0xFF, (ip >> 24) & 0xFF);
    return buf;
}

// Helper for TCP state to string
const char* TcpStateToString(DWORD state) {
    switch (state) {
    case MIB_TCP_STATE_ESTAB: return "ESTABLISHED";
    case MIB_TCP_STATE_LISTEN: return "LISTEN";
    default: return "OTHER";
    }
}

// Test WinINet: InternetConnect + HttpOpenRequest/HttpSendRequest
void Networking::TestInternetConnect() {
    HINTERNET hInternet = InternetOpen(L"Test", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    bool success = (hInternet != NULL);
    if (success) {
        HINTERNET hConnect = InternetConnect(hInternet, L"example.com", INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
        success = (hConnect != NULL);
        if (success) {
            HINTERNET hRequest = HttpOpenRequest(hConnect, L"GET", L"/", NULL, NULL, NULL, 0, 0);
            if (hRequest) {
                success = HttpSendRequest(hRequest, NULL, 0, NULL, 0);
                InternetCloseHandle(hRequest);
            } else {
                success = false;
            }
            InternetCloseHandle(hConnect);
        }
        InternetCloseHandle(hInternet);
    }
    PrintTestResult(L"InternetConnect", success, success ? L"Connected to example.com and sent GET" : L"Failed: " + std::to_wstring(GetLastError()));
}

// Test WinINet: InternetReadFile (download to temp file)
void Networking::TestInternetReadFile() {
    HINTERNET hInternet = InternetOpen(L"Test", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    bool success = (hInternet != NULL);
    if (success) {
        HINTERNET hUrl = InternetOpenUrl(hInternet, L"http://example.com", NULL, 0, INTERNET_FLAG_RELOAD, 0);
        if (hUrl) {
            char buffer[4096];
            DWORD bytesRead;
            std::wstring tempFile = L"C:\\temp\\test_read.txt";
            std::ofstream outFile(tempFile, std::ios::binary);
            
            // BUG FIX #2: Check if file opened successfully
            if (!outFile.is_open()) {
                InternetCloseHandle(hUrl);
                InternetCloseHandle(hInternet);
                PrintTestResult(L"InternetReadFile", false, L"Failed to open temp file");
                return;
            }
            
            while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
                outFile.write(buffer, bytesRead);
            }
            outFile.close();
            success = std::ifstream(tempFile).good();
            DeleteFileW(tempFile.c_str());
            InternetCloseHandle(hUrl);
        } else {
            success = false;  // BUG FIX #3: Set success to false if InternetOpenUrl fails
        }
        InternetCloseHandle(hInternet);
    }
    PrintTestResult(L"InternetReadFile", success, success ? L"Downloaded example.com to temp file" : L"Failed: " + std::to_wstring(GetLastError()));
}

// Test IP Helper: GetAdaptersInfo
void Networking::TestGetAdaptersInfo() {
    ULONG size = 0;
    GetAdaptersInfo(NULL, &size);
    // BUG FIX #4: Don't check GetLastError() before calling the function properly
    if (size == 0) {
        PrintTestResult(L"GetAdaptersInfo", false, L"Failed to get buffer size");
        return;
    }
    
    std::vector<BYTE> buffer(size);
    PIP_ADAPTER_INFO pAdapter = reinterpret_cast<PIP_ADAPTER_INFO>(buffer.data());
    bool success = (GetAdaptersInfo(pAdapter, &size) == NO_ERROR);
    
    if (success) {
        int count = 0;
        PIP_ADAPTER_INFO pCurrent = pAdapter;  // BUG FIX #5: Don't modify pAdapter, use separate pointer
        while (pCurrent) {
            count++;
            pCurrent = pCurrent->Next;
        }
        PrintTestResult(L"GetAdaptersInfo", true, L"Found " + std::to_wstring(count) + L" adapters");
    } else {
        PrintTestResult(L"GetAdaptersInfo", false, L"Failed: " + std::to_wstring(GetLastError()));
    }
}

// Test IP Helper: GetIpNetTable (ARP table)
void Networking::TestGetIpNetTable() {
    ULONG size = 0;
    GetIpNetTable(NULL, &size, FALSE);
    // BUG FIX #6: Same issue - don't rely on GetLastError before proper call
    if (size == 0) {
        PrintTestResult(L"GetIpNetTable", false, L"Failed to get buffer size");
        return;
    }
    
    std::vector<BYTE> buffer(size);
    PMIB_IPNETTABLE pTable = reinterpret_cast<PMIB_IPNETTABLE>(buffer.data());
    bool success = (GetIpNetTable(pTable, &size, FALSE) == NO_ERROR);
    
    if (success) {
        PrintTestResult(L"GetIpNetTable", true, L"ARP table: " + std::to_wstring(pTable->dwNumEntries) + L" entries");
    } else {
        PrintTestResult(L"GetIpNetTable", false, L"Failed: " + std::to_wstring(GetLastError()));
    }
}

// Test IP Helper: ICMP Echo (ping)
void Networking::TestIcmpEchoRequest() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    HANDLE hIcmp = IcmpCreateFile();
    bool success = (hIcmp != INVALID_HANDLE_VALUE);
    if (success) {
        // BUG FIX #7: Buffer size too small - need proper size for ICMP_ECHO_REPLY
        std::vector<BYTE> reply(sizeof(ICMP_ECHO_REPLY) + 32); // Add padding for data
        char sendData[32] = "ping test";  // BUG FIX #8: Need actual data to send
        DWORD ret = IcmpSendEcho(hIcmp, inet_addr("8.8.8.8"), sendData, sizeof(sendData), NULL, reply.data(), (DWORD)reply.size(), 1000);
        success = (ret > 0);
        IcmpCloseHandle(hIcmp);
    }
    WSACleanup();
    PrintTestResult(L"IcmpEchoRequest", success, success ? L"Ping to 8.8.8.8 succeeded" : L"Ping failed");
}

// Test IP Helper: GetExtendedTcpTable (process connections)
void Networking::TestGetExtendedTcpTable() {
    ULONG size = 0;
    GetExtendedTcpTable(NULL, &size, TRUE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0);
    // BUG FIX #9: Same pattern issue
    if (size == 0) {
        PrintTestResult(L"GetExtendedTcpTable", false, L"Failed to get buffer size");
        return;
    }
    
    std::vector<BYTE> buffer(size);
    PMIB_TCPTABLE_OWNER_PID pTable = reinterpret_cast<PMIB_TCPTABLE_OWNER_PID>(buffer.data());
    bool success = (GetExtendedTcpTable(pTable, &size, TRUE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0) == NO_ERROR);
    
    if (success) {
        int count = 0;
        for (DWORD i = 0; i < pTable->dwNumEntries; ++i) {
            if (pTable->table[i].dwOwningPid > 0) count++;
        }
        PrintTestResult(L"GetExtendedTcpTable", true, L"Found " + std::to_wstring(count) + L" TCP connections with PIDs");
    } else {
        PrintTestResult(L"GetExtendedTcpTable", false, L"Failed: " + std::to_wstring(GetLastError()));
    }
}

// Test WinHTTP: WinHttpConnect
void Networking::TestWinHttpConnect() {
    HINTERNET hSession = WinHttpOpen(L"Test", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    bool success = (hSession != NULL);
    if (success) {
        HINTERNET hConnect = WinHttpConnect(hSession, L"example.com", INTERNET_DEFAULT_HTTP_PORT, 0);
        success = (hConnect != NULL);
        if (success) {
            HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", L"/", NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
            if (hRequest) {
                success = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
                if (success) success = WinHttpReceiveResponse(hRequest, NULL);
                WinHttpCloseHandle(hRequest);
            } else {
                success = false;  // BUG FIX #10: Set success to false if WinHttpOpenRequest fails
            }
            WinHttpCloseHandle(hConnect);
        }
        WinHttpCloseHandle(hSession);
    }
    PrintTestResult(L"WinHttpConnect", success, success ? L"Connected via WinHTTP" : L"Failed: " + std::to_wstring(GetLastError()));
}

// Test Windows Sockets: connect (TCP to example.com)
void Networking::TestSocketConnect() {
    WSADATA wsaData;
    // BUG FIX #11: Check WSAStartup result
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        PrintTestResult(L"SocketConnect", false, L"WSAStartup failed");
        return;
    }
    
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    bool success = (sock != INVALID_SOCKET);
    if (success) {
        sockaddr_in addr = {};  // BUG FIX #12: Initialize structure
        addr.sin_family = AF_INET;
        addr.sin_port = htons(80);
        inet_pton(AF_INET, "93.184.216.34", &addr.sin_addr);  // example.com IP
        
        // BUG FIX #13: Set socket timeout to avoid hanging
        DWORD timeout = 3000; // 3 seconds
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
        
        success = (connect(sock, (sockaddr*)&addr, sizeof(addr)) != SOCKET_ERROR);
        closesocket(sock);
    }
    WSACleanup();
    PrintTestResult(L"SocketConnect", success, success ? L"TCP connected to example.com:80" : L"Connect failed: " + std::to_wstring(WSAGetLastError()));
}

// Test Windows Sockets: GetAddrInfo
void Networking::TestGetAddrInfo() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        PrintTestResult(L"GetAddrInfo", false, L"WSAStartup failed");
        return;
    }
    
    addrinfo* result = NULL;
    addrinfo hints = {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    bool success = (getaddrinfo("example.com", "80", &hints, &result) == 0);
    
    if (success && result) {
        freeaddrinfo(result);  // BUG FIX #14: Free before PrintTestResult
        PrintTestResult(L"GetAddrInfo", true, L"Resolved example.com");
    } else {
        PrintTestResult(L"GetAddrInfo", false, L"Failed: " + std::to_wstring(WSAGetLastError()));
    }
    
    WSACleanup();
}

// Test Windows Sockets: gethostname
void Networking::TestGetHostName() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        PrintTestResult(L"GetHostName", false, L"WSAStartup failed");
        return;
    }
    
    char hostname[256];
    bool success = (gethostname(hostname, sizeof(hostname)) == 0);
    WSACleanup();
    
    // BUG FIX #15: Proper string conversion from char* to wstring
    if (success) {
        std::string hostnameStr(hostname);
        std::wstring hostnameW(hostnameStr.begin(), hostnameStr.end());
        PrintTestResult(L"GetHostName", true, L"Hostname: " + hostnameW);
    } else {
        PrintTestResult(L"GetHostName", false, L"Failed: " + std::to_wstring(WSAGetLastError()));
    }
}

// Test Windows Sockets: listen (local server on 8080)
void Networking::TestSocketListen() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        PrintTestResult(L"SocketListen", false, L"WSAStartup failed");
        return;
    }
    
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    bool success = (sock != INVALID_SOCKET);
    if (success) {
        sockaddr_in addr = {};  // BUG FIX #16: Initialize structure
        addr.sin_family = AF_INET;
        addr.sin_port = htons(8080);
        addr.sin_addr.s_addr = INADDR_ANY;
        success = (bind(sock, (sockaddr*)&addr, sizeof(addr)) != SOCKET_ERROR);
        if (success) {
            success = (listen(sock, SOMAXCONN) != SOCKET_ERROR);
        }
        closesocket(sock);
    }
    WSACleanup();
    PrintTestResult(L"SocketListen", success, success ? L"Listening on port 8080" : L"Bind/Listen failed: " + std::to_wstring(WSAGetLastError()));
}

// Test Windows Sockets: Port Scanning (scan common ports on 8.8.8.8)
void Networking::TestPortScanning() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        PrintTestResult(L"PortScanning", false, L"WSAStartup failed");
        return;
    }
    
    int openPorts = 0;
    int ports[] = {21, 22, 53, 80, 135, 443, 445, 3306, 3389, 5985};
    
    for (int port : ports) {
        SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET) continue;  // BUG FIX #17: Check socket creation
        
        sockaddr_in addr = {};  // BUG FIX #18: Initialize structure
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, "8.8.8.8", &addr.sin_addr);
        
        // BUG FIX #19: Set very short timeout for port scanning
        DWORD timeout = 500; // 500ms
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
        
        if (connect(sock, (sockaddr*)&addr, sizeof(addr)) != SOCKET_ERROR) {
            openPorts++;
        }
        closesocket(sock);
    }
    WSACleanup();
    PrintTestResult(L"PortScanning", true, L"Scanned 10 ports on 8.8.8.8: " + std::to_wstring(openPorts) + L" open");
}