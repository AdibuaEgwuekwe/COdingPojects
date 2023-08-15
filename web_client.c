#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8000
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <file_path>\n", argv[0]);
        exit(1);
    }

    char *file_path = argv[1];

    // Initialize Winsock
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2, 2), &wsa_data);

    // Set up the client socket
    SOCKET client_socket;
    struct sockaddr_in server_address;

    client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = htons(PORT);

    // Connect to the server
    connect(client_socket, (SOCKADDR *)&server_address, sizeof(server_address));

    // Send an HTTP GET request
    char request[BUFFER_SIZE];
    snprintf(request, BUFFER_SIZE, "GET %s HTTP/1.0\r\n\r\n", file_path);
    send(client_socket, request, strlen(request), 0);

    // Receive the response from the server
    char response[BUFFER_SIZE];
    recv(client_socket, response, BUFFER_SIZE, 0);

    // Print the server's response
    printf("%s\n", response);

    // Close the client socket and clean up Winsock resources
    closesocket(client_socket);
    WSACleanup();

    return 0;
}
