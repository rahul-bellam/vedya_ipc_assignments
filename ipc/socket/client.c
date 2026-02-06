#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in serv_addr;
    char sendbuf[BUFFER_SIZE];
    char recvbuf[BUFFER_SIZE + 1];
    const char *server_ip = "127.0.0.1";   // default
    if (argc >= 2) server_ip = argv[1];    // allow override: ./client <ip>
    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }
    // Setup server address
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock);
        return 1;
    }
    // Connect
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        close(sock);
        return 1;
    }
    printf("Connected to server %s:%d\n", server_ip, PORT);
    printf("Type a message and press Enter (type 'exit' to quit)\n");
    while (1) {
        printf("> ");
        fflush(stdout);
        if (!fgets(sendbuf, sizeof(sendbuf), stdin)) {
            printf("\nEOF detected. Exiting...\n");
            break;
        }
        // Remove trailing newline
        sendbuf[strcspn(sendbuf, "\n")] = '\0';
        if (strcmp(sendbuf, "exit") == 0) {
            printf("Closing connection...\n");
            break;
        }
        // Send message
        if (send(sock, sendbuf, strlen(sendbuf), 0) < 0) {
            perror("send");
            break;
        }
        // Receive server response
        int n = recv(sock, recvbuf, BUFFER_SIZE, 0);
        if (n == 0) {
            printf("Server closed the connection.\n");
            break;
        } else if (n < 0) {
            perror("recv");
            break;
        }
        recvbuf[n] = '\0';
        printf("Server: %s\n", recvbuf);
    }
    close(sock);
    return 0;
}
