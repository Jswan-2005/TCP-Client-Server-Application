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
#include <mutex>


#pragma comment(lib, "Ws2_32.lib")

struct Message{
    char buffer[200]{};
    char user[50]{};
};

void receiveMessages(SOCKET clientSocket) {
    while (true) {
        Message receivedMsg;
        int rec = recv(clientSocket,(char*)&receivedMsg,sizeof(receivedMsg),0);
        if (rec == SOCKET_ERROR) {
            closesocket(clientSocket);
            WSACleanup();
        }
        else {
            std::cout << std::endl;
            std::cout << receivedMsg.user << "| " << receivedMsg.buffer << std::endl;
        };
    }
}

void handleClient(SOCKET clientSocket, char username[]){
    std::cout << "Enter your message ";
    while (true) {
        Message msg;
        strncpy(msg.user, username, sizeof(msg.user));
        msg.user[sizeof(msg.user) - 1] = '\0';
        std::cin.getline(msg.buffer, sizeof(msg.buffer));
        int byteCount = send(clientSocket, (char*)&msg, sizeof(msg),0);
        if (byteCount == SOCKET_ERROR) {
            closesocket(clientSocket);
            WSACleanup();
        }
    }
};

    int main() {
    WSADATA wsaData;
    int wsaerr;
    WORD wVersionRequested = MAKEWORD(2,2);
    wsaerr = WSAStartup(wVersionRequested, &wsaData);
    if (wsaerr !=0) {
        std::cout << "The Winsock dll not found" << std::endl;
        return 0;
    }

    SOCKET clientSocket = INVALID_SOCKET;
    clientSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        std::cout << "Error at socket(): " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 0;
    }


    sockaddr_in clientService;
    clientService.sin_family = AF_INET;
    InetPton(AF_INET, ("127.0.0.1"),&clientService.sin_addr.s_addr);
    clientService.sin_port = htons(8080);
    if (connect(clientSocket, (SOCKADDR*)&clientService,sizeof(clientService))==SOCKET_ERROR) {
        std::cout << "connect failed" << std::endl;
        WSACleanup();
        return 0;
    }
    ;
    char username[200];
    std::cout << "What is your username" << std::endl;
    std::cin >> username;
    std::thread t1(handleClient,clientSocket,username);
    t1.detach();
    std::thread t2(receiveMessages, clientSocket);
    t2.join();
}

