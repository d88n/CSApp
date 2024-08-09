#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

int __cdecl main(int argc, char **argv) 
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL, *ptr = NULL, hints;

    char sendbuf[DEFAULT_BUFLEN];
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;

    if (argc < 2) {
        printf("Usage: %s server-name [port]\n", argv[0]);
        return 1;
    }

    const char *port = (argc == 3) ? argv[2] : DEFAULT_PORT;

    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    iResult = getaddrinfo(argv[1], port, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    for(ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            printf("connect failed with error: %ld\n", WSAGetLastError());
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    int choice;
    printf("Choose an option:\n1. Send a text message\n2. Send two integers to add\n");
    scanf("%d", &choice);
    getchar();

    if (choice == 1) {
        printf("Enter the message to send: ");
        fgets(sendbuf, DEFAULT_BUFLEN, stdin);
        sendbuf[strcspn(sendbuf, "\n")] = 0;

        iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
        if (iResult == SOCKET_ERROR) {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
            return 1;
        }
        printf("Message Sent: %s\n", sendbuf);
    } 
    else if (choice == 2) {
        int num1, num2;
        printf("Enter the first number: ");
        scanf("%d", &num1);
        printf("Enter the second number: ");
        scanf("%d", &num2);

        snprintf(sendbuf, DEFAULT_BUFLEN, "NUMBERS %d %d", num1, num2);

        iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
        if (iResult == SOCKET_ERROR) {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
            return 1;
        }
        printf("Numbers Sent: %s\n", sendbuf);
    } else {
        printf("Invalid choice\n");
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // Receive the response from the server
    iResult = recv(ConnectSocket, recvbuf, recvbuflen-1, 0);
    if (iResult > 0) {
        recvbuf[iResult] = '\0';
        printf("Message received: %s\n", recvbuf);
    }
    else if (iResult == 0) {
        printf("Connection closed by server\n");
    }
    else {
        printf("recv failed with error: %d\n", WSAGetLastError());
    }

    // Close the connection
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}
