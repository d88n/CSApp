#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"


void handle_client(SOCKET ClientSocket) {
    int iResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    char sendbuf[DEFAULT_BUFLEN];

    while (true) {
        // Receive data from the client
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            recvbuf[iResult] = '\0';
            printf("Message received: %s\n", recvbuf);

            // Handle different types of messages
            int num1, num2;

            if (sscanf(recvbuf, "NUMBERS %d %d", &num1, &num2) == 2) {
                snprintf(sendbuf, DEFAULT_BUFLEN, "The sum of two ints is: %d", num1 + num2);
            } else if (strcmp(recvbuf, "exit") == 0) {
                printf("Exit command received. Closing connection...\n");
                break; // Exit the loop to close the connection
            } else {
                // Echo the message back to the client
                snprintf(sendbuf, DEFAULT_BUFLEN, "%s", recvbuf);
            }

            // Send data back to the client
            iResult = send(ClientSocket, sendbuf, (int)strlen(sendbuf), 0);
            if (iResult == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                break;
            }
        } else if (iResult == 0) {
            printf("Connection closing...\n");
            break;
        } else {
            printf("recv failed with error: %d\n", WSAGetLastError());
            break;
        }
    }

    // Cleanup and close the client socket
    shutdown(ClientSocket, SD_SEND);
    closesocket(ClientSocket);
    printf("Client disconnected.\n");
}

int __cdecl main(int argc, char **argv) {
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL;
    struct addrinfo hints;

    const char *port = (argc == 2) ? argv[1] : DEFAULT_PORT;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    // Set up the hints structure for IPv4
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET; // Use AF_INET to support IPv4 only
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, port, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    printf("Server is listening on port %s\n", port);

    // Main loop to accept and handle client connections
    while (true) {
        SOCKET ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }

        printf("Client connected.\n");

        // Create a new thread to handle the client
        std::thread client_thread(handle_client, ClientSocket);
        client_thread.detach(); // Detach the thread to allow independent execution
    }

    // Cleanup
    closesocket(ListenSocket);
    WSACleanup();

    return 0;
}
