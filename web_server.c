#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8000
#define BUFFER_SIZE 1024

char root_path[BUFFER_SIZE];

// Function to handle the client request
void *handle_client(void *arg) {
    // Cast the argument to a SOCKET
    SOCKET client_socket = *(SOCKET *)arg;
    char buffer[BUFFER_SIZE];
    char file_path[BUFFER_SIZE];

    // Receive the request from the client
    recv(client_socket, buffer, BUFFER_SIZE, 0);

    // Extract the file path from the received request
    sscanf(buffer, "GET %s HTTP", file_path);

    // Concatenate the root path and the requested file path
    char full_path[BUFFER_SIZE];
    snprintf(full_path, BUFFER_SIZE, "%s%s", root_path, file_path);

    // Try to open the requested file
    int file = _open(full_path, _O_RDONLY);
    int status_code;
    char response[BUFFER_SIZE * 2];

    // If the file exists, build a 200 OK response with the file content
    if (file != -1) {
        status_code = 200;
        off_t file_size = _lseek(file, 0, SEEK_END);
        _lseek(file, 0, SEEK_SET);

        char file_content[file_size];
        _read(file, file_content, file_size);

        snprintf(response, BUFFER_SIZE * 2, "HTTP/1.0 %d OK\r\nContent-Length: %ld\r\n\r\n%s", status_code, file_size, file_content);
    } else {
        // If the file does not exist, build a 404 File Not Found response with an error message
        status_code = 404;
        char *error_message = "File Not Found";
        snprintf(response, BUFFER_SIZE * 2, "HTTP/1.0 %d %s\r\nContent-Length: %ld\r\n\r\n%s", status_code, error_message, strlen(error_message), error_message);
    }

    // Send the response to the client
    send(client_socket, response, strlen(response), 0);

    // Close the file (if opened) and the connected socket
    if (file != -1) {
        _close(file);
    }
    closesocket(client_socket);

    // Exit the thread function
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    // Check for the correct number of command-line arguments
    if (argc != 2) {
        printf("Usage: %s <path>\n", argv[0]);
        exit(1);
    }

    // Store the given path to serve files
    strcpy(root_path, argv[1]);

    // Initialize Winsock
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2, 2), &wsa_data);

    // Set up the server socket
    SOCKET server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    int client_address_len = sizeof(client_address);

    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    server_address.sin_family = AF_INET;
        server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Bind the server socket to the address and port
    bind(server_socket, (SOCKADDR *)&server_address, sizeof(server_address));

    // Listen for incoming connections
    listen(server_socket, 5);

    // Enter an infinite loop to process incoming requests
    while (1) {
        // Accept a new client connection
        client_socket = accept(server_socket, (SOCKADDR *)&client_address, &client_address_len);

        // Create a new thread to handle the client request
        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, (void *)&client_socket);

        // Detach the thread to avoid memory leaks
        pthread_detach(thread);
    }

    // Close the server socket and clean up Winsock resources
    closesocket(server_socket);
    WSACleanup();

    return 0;
}
