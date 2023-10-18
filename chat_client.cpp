#include <iostream>
#include <string>
#include <winsock2.h>
#include <thread>
#include<vector>
#include <ws2tcpip.h>  // Required for inet_pton
#pragma comment(lib, "ws2_32.lib")

#define MAX_BUFFER 2048

SOCKET sockClient;

std::vector<std::string> messageHistory;

void addToHistory(const std::string& message) {
    messageHistory.push_back(message);
}

void showHistory() {
    for (const auto& msg : messageHistory) {
        std::cout << msg << std::endl;
    }
}

void receiveMessages() {
    char recvBuf[MAX_BUFFER];
    while (true) {
        memset(recvBuf, 0, sizeof(recvBuf));
        int bytesRecv = recv(sockClient, recvBuf, MAX_BUFFER - 1, 0);
        if (bytesRecv <= 0) {
            std::cerr << "Connection closed or an error occurred!" << std::endl;
            closesocket(sockClient);
            WSACleanup();
            exit(-1);
        }
        addToHistory(recvBuf);
        std::cout << recvBuf << std::endl;
    }
}

int main() {
    WSADATA wsaData;
    int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (ret != 0) {
        std::cerr << "WSAStartup failed!" << std::endl;
        return -1;
    }

    sockClient = socket(AF_INET, SOCK_STREAM, 0);
    if (sockClient == INVALID_SOCKET) {
        std::cerr << "Failed to create socket!" << std::endl;
        WSACleanup();
        return -1;
    }

    SOCKADDR_IN addrSrv;
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(5000);  // Server is expected to be listening on port 5000

    // Use inet_pton instead of inet_addr to avoid deprecation warning
    if (inet_pton(AF_INET, "127.0.0.1", &addrSrv.sin_addr) <= 0) {
        std::cerr << "inet_pton error!" << std::endl;
        closesocket(sockClient);
        WSACleanup();
        return -1;
    }

    if (connect(sockClient, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR)) == SOCKET_ERROR) {
        std::cerr << "Connect error!" << std::endl;
        closesocket(sockClient);
        WSACleanup();
        return -1;
    }

    // Start a new thread to receive messages from the server
    std::thread recvThread(receiveMessages);
    recvThread.detach();

    std::string message;
    while (true) {
        std::getline(std::cin, message);  // Get input from the user
        if (message == "exit") {  // Exit condition
            break;
        }
        if (message == "history") {
            showHistory();
            continue;
        }
        send(sockClient, message.c_str(), message.length(), 0);
        std::string a = "You:";
        addToHistory(a + message);
        std::cout << "You: " << message << std::endl;
    }

    // Cleanup
    closesocket(sockClient);
    WSACleanup();

    return 0;
}
