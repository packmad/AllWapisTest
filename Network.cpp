#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include <iostream>
#include <cstring>
#include <iostream>
#include <wininet.h>
#include <windns.h>   //DNS api's
#include <stdio.h>    //standard i/o
#include <winsock.h>  //winsock
#include "MainHeader.h"
#pragma comment(lib,"wininet.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "dnsapi.lib")

using namespace std;

int socketconnect() {
    WSADATA wsa;
    SOCKET s;
    struct sockaddr_in server;

    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Failed. Error Code : %d", WSAGetLastError());
        return 1;
    }

    printf("Initialised.\n");

    //Create a socket
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        printf("Could not create socket : %d", WSAGetLastError());
    }

    printf("Socket created.\n");


    server.sin_addr.s_addr = inet_addr("213.209.17.209"); // libero.it
    server.sin_family = AF_INET;
    server.sin_port = htons(80);
    printf("%d", ntohs(server.sin_port));
    printf("%d", htons(server.sin_port));

    char* strAdd2 = inet_ntoa(server.sin_addr);
    puts(strAdd2);
    //Connect to remote server
    if (connect(s, (struct sockaddr*)&server, sizeof(server)) < 0)
    {
        puts("connect error");
        return 1;
    }

    puts("Connected");

    return 0;
}


unsigned long inetadd() {
    unsigned long ulAddr = INADDR_NONE;
    const char* a = "8.8.8.8";
    ulAddr = inet_addr(a);
    if (ulAddr == INADDR_NONE) {
        printf("inet_addr failed and returned INADDR_NONE\n");
        WSACleanup();
        return 1;
    }
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

   
    int sock;
    char* hostAddress;
    struct addrinfo hints, * res;
    int err;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    const char* h = "diocane.bau";
    getaddrinfo(h, NULL, &hints, &res);
    
}


void dns() {
    PIP4_ARRAY pSrvList = (PIP4_ARRAY)LocalAlloc(LPTR, sizeof(IP4_ARRAY));
    PDNS_RECORD pDnsRecord;
    DNS_STATUS status;
    
    string u = "unige.it";
    PCSTR d = u.c_str();
    PCWSTR d2 = L"unige.it";
    status = DnsQuery_W(d2,                 //pointer to OwnerName 
        DNS_TYPE_A,                      //Type of the record to be queried
        DNS_QUERY_BYPASS_CACHE,     // Bypasses the resolver cache on the lookup. 
        pSrvList,                   //contains DNS server IP address
        &pDnsRecord,                //Resource record comprising the response
        NULL);
    status = DnsQuery_A(d,                 //pointer to OwnerName 
        DNS_TYPE_A,                      //Type of the record to be queried
        DNS_QUERY_BYPASS_CACHE,     // Bypasses the resolver cache on the lookup. 
        pSrvList,                   //contains DNS server IP address
        &pDnsRecord,                //Resource record comprising the response
        NULL);
}

void connect_sendrequest() {
    HINTERNET hSession = InternetOpen(
        L"Mozilla/5.0",
        INTERNET_OPEN_TYPE_PRECONFIG,
        NULL,
        NULL,
        0);

    HINTERNET hConnect = InternetConnect(
        hSession,
        L"www.google.it",
        INTERNET_DEFAULT_HTTPS_PORT, // THIS
        L"",
        L"",
        INTERNET_SERVICE_HTTP,
        0,
        0);

    HINTERNET hHttpFile = HttpOpenRequest(
        hConnect,
        L"GET",
        L"/",
        NULL,
        NULL,
        NULL,
        INTERNET_FLAG_SECURE, // THIS
        0);

    while (!HttpSendRequest(hHttpFile, NULL, 0, 0, 0)) {
        printf("HttpSendRequest error : (%lu)\n", GetLastError());

        InternetErrorDlg(
            GetDesktopWindow(),
            hHttpFile,
            ERROR_INTERNET_CLIENT_AUTH_CERT_NEEDED,
            FLAGS_ERROR_UI_FILTER_FOR_ERRORS |
            FLAGS_ERROR_UI_FLAGS_GENERATE_DATA |
            FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS,
            NULL);
    }

    DWORD dwFileSize;
    dwFileSize = BUFSIZ;

    char* buffer;
    buffer = new char[dwFileSize + 1];

    while (true) {
        DWORD dwBytesRead;
        BOOL bRead;

        bRead = InternetReadFile(
            hHttpFile,
            buffer,
            dwFileSize + 1,
            &dwBytesRead);

        if (dwBytesRead == 0) break;

        if (!bRead) {
            printf("InternetReadFile error : <%lu>\n", GetLastError());
        }
        else {
            buffer[dwBytesRead] = 0;
            //printf("Retrieved %lu data bytes: %s\n", dwBytesRead, buffer);
        }
    }

    InternetCloseHandle(hHttpFile);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hSession);

}

int network() {
    socketconnect();
    inetadd();
    dns();
    connect_sendrequest();


    LPCWSTR userAgent = L"TestWindow";
    LPCWSTR tgtUrl = L"http://www.google.com";


    HINTERNET connect = InternetOpen(userAgent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

    if (!connect) {
        std::cout << "Connection Failed or Syntax error";
        return 0;
    }

    HINTERNET OpenAddress = InternetOpenUrl(connect, tgtUrl, NULL, 0, INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_KEEP_CONNECTION, 0);

    if (!OpenAddress)
    {
        DWORD ErrorNum = GetLastError();
        std::cout << "Failed to open URL \nError No: " << ErrorNum;
        InternetCloseHandle(connect);
        return 0;
    }

    char DataReceived[4096];
    DWORD NumberOfBytesRead = 0;
    int n = 0;
    while (InternetReadFile(OpenAddress, DataReceived, 4096, &NumberOfBytesRead) && NumberOfBytesRead)
    {
        //std::cout << DataReceived;
        n++;
    }

    InternetCloseHandle(OpenAddress);
    InternetCloseHandle(connect);

    return 0;
}