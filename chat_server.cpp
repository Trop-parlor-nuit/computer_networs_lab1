#include <iostream>
#include <string>
#include <map>
#include <thread>
#include <mutex>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define MAX_BUFFER 2048

std::map<SOCKET, int> clientMap;  // socket -> clientId
std::mutex clientMutex;
int clientIdCounter = 0;

// 将消息广播到所有客户端，除了发送者
void broadcastMessage(const std::string& message, SOCKET sender) {
    std::lock_guard<std::mutex> lock(clientMutex);
    for (const auto& client : clientMap) {
        if (client.first != sender) {
            send(client.first, message.c_str(), message.length(), 0);
        }
    }
}

DWORD WINAPI handlerRequest(LPVOID lparam) {
    SOCKET clientSocket = (SOCKET)lparam;
    int clientId;
    {
        std::lock_guard<std::mutex> lock(clientMutex);
        clientId = ++clientIdCounter;
        clientMap[clientSocket] = clientId;
    }

    char recvBuf[MAX_BUFFER];

    while (true) {
        memset(recvBuf, 0, sizeof(recvBuf));
        int bytesRecv = recv(clientSocket, recvBuf, MAX_BUFFER - 1, 0);
        if (bytesRecv <= 0) {
            std::cerr << "Connection with Client " << clientId << " closed or an error occurred!" << std::endl;
            break;
        }

        std::string forwardMsg = "Client " + std::to_string(clientId) + ": " + recvBuf;
        broadcastMessage(forwardMsg, clientSocket);
    }

    closesocket(clientSocket);
    {
        std::lock_guard<std::mutex> lock(clientMutex);
        clientMap.erase(clientSocket);
    }

    return 0;
}

int main() {
    WSADATA wsaData;
    int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (ret != 0) {
        std::cerr << "WSAStartup failed!" << std::endl;
        return -1;
    }

    SOCKET sockSrv = socket(AF_INET, SOCK_STREAM, 0);
    if (sockSrv == INVALID_SOCKET) {
        std::cerr << "Failed to create socket!" << std::endl;
        WSACleanup();
        return -1;
    }

    SOCKADDR_IN addrSrv;
    addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(5000);  // Choosing port 5000 for the example

    if (bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR)) == SOCKET_ERROR) {
        std::cerr << "Bind error!" << std::endl;
        closesocket(sockSrv);
        WSACleanup();
        return -1;
    }

    if (listen(sockSrv, 5) == SOCKET_ERROR) {
        std::cerr << "Listen error!" << std::endl;
        closesocket(sockSrv);
        WSACleanup();
        return -1;
    }

    std::cout << "Server running and waiting for clients..." << std::endl;

    while (true) {
        SOCKADDR_IN addrClient;
        int len = sizeof(SOCKADDR);
        SOCKET sockConn = accept(sockSrv, (SOCKADDR*)&addrClient, &len);
        if (sockConn == INVALID_SOCKET) {
            std::cerr << "Accept error!" << std::endl;
            continue;  // Accept the next connection
        }

        std::thread handlerThread(handlerRequest, (LPVOID)sockConn);
        handlerThread.detach();  // Let the thread run freely
    }

    // Cleanup
    closesocket(sockSrv);
    WSACleanup();

    return 0;
}
