#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <iostream>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <cstdio>
#include <thread>
#include <chrono>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")

std::mutex vecMtx;

struct Message{
    char buffer[200]{};
    char user[50]{};
};

void printReceivedMessage(SOCKET acceptSocket, std::vector<SOCKET> currentConnections) {
    while (true) {
        Message recievedMsg;
        int rec = recv(acceptSocket, (char*)&recievedMsg,sizeof(recievedMsg),0);
        if (rec == SOCKET_ERROR) {
            std::lock_guard lck(vecMtx);
            std::erase(currentConnections,acceptSocket);
            return;
        }
        std::cout << recievedMsg.user << "| " << recievedMsg.buffer << std::endl;
    }
};

void printCurrentConnection(std::vector<SOCKET> &currentConnections) {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        std::cout << "Current connections " << currentConnections.size() << std::endl;
    }
}

int main() {
    WSADATA wsaData;
    int wsaerr;
    WORD wVersionRequested = MAKEWORD(2,2);
    wsaerr = WSAStartup(wVersionRequested, &wsaData);
    if (wsaerr != 0) {
        std::cout << "Winsock dll not found" << std::endl;
        return 0;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cout << "Error creating server socket" << std::endl;
        WSACleanup();
        return 0;
    }

    sockaddr_in service;
    service.sin_family = AF_INET;
    InetPton(AF_INET, "127.0.0.1", &service.sin_addr.s_addr);
    service.sin_port = htons(8080);
    if (bind(serverSocket,(SOCKADDR*)&service,sizeof(service))==SOCKET_ERROR) {
        std::cout << "bind failed" << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 0;
    }

    if (listen(serverSocket,5) == SOCKET_ERROR) {
        std::cout << "listen failed" << std::endl;
    }

    std::vector<SOCKET> currentConnections;
    std::thread curConnectionThread(printCurrentConnection, std::ref(currentConnections));
    curConnectionThread.detach();
    while (true) {
        SOCKET acceptSocket = accept(serverSocket,nullptr,nullptr);
        if (acceptSocket == INVALID_SOCKET) {
            std::cout << "accept failed" << std::endl;
            closesocket(serverSocket);
            closesocket(acceptSocket);
            WSACleanup();
            return 0;
        }
        currentConnections.push_back(acceptSocket);
        std::thread recvThread(printReceivedMessage,acceptSocket, std::ref(currentConnections));
        recvThread.detach();
    }
}



