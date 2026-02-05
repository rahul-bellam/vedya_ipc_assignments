#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
 
int main() {
    int sock;
    struct sockaddr_in server;
    char msg[] = "Hello Server";
    char buffer[1024];
    int valread;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    connect(sock, (struct sockaddr *)&server, sizeof(server));

    send(sock, msg, strlen(msg), 0);
// Wait for messages from server until connection is closed
    while ((valread = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
         buffer[valread] = '\0';
         printf("Server: %s\n", buffer);
        // Optionally, break if a specific close message is received
        if (strstr(buffer, "Connection terminated by server") != NULL) {
            break;
        }
     }
 
    // Wait for user input before exiting to keep connection open
    printf("Press Enter to exit...\n");
    getchar();
     return 0;
 }