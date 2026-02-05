#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
 
#define PORT 8080
#define MAX_CLIENTS 5
#define BUFFER_SIZE 1024
 
int main() {
    int server_fd, new_socket;
    int client_socket[MAX_CLIENTS] = {0};
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];
 
    fd_set readfds;
 
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
 
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
 
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, MAX_CLIENTS);
 
    printf("Server listening on port %d\n", PORT);
 
    while (1) {
       FD_ZERO(&readfds);
       FD_SET(server_fd, &readfds);
       int max_sd = server_fd;
 
        // add client sockets to set
     for (int i = 0; i < MAX_CLIENTS; i++) {
       int sd = client_socket[i];
        if (sd > 0)
         FD_SET(sd, &readfds);
        if (sd > max_sd)
          max_sd = sd;
        }
 
        // wait for activity
        select(max_sd + 1, &readfds, NULL, NULL, NULL);
 
        // new connection
        if (FD_ISSET(server_fd, &readfds)) {
            new_socket = accept(server_fd,
                                (struct sockaddr *)&address,
                                &addrlen);
 
            printf("New client connected: socket fd %d\n", new_socket);
 
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_socket[i] == 0) {
                    client_socket[i] = new_socket;
                    break;
                }
            }
        }
 
        // check clients
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = client_socket[i];
 
            if (FD_ISSET(sd, &readfds)) {
                int valread = recv(sd, buffer, BUFFER_SIZE, 0);
 
                if (valread == 0) {
                    // client disconnected
                    const char *term_msg = "Connection terminated by server";
                    send(sd, term_msg, strlen(term_msg), 0);
                    printf("Client on socket %d disconnected\n", sd);
                    close(sd);
                    client_socket[i] = 0;
                } else {
                    buffer[valread] = '\0';
                    printf("Client %d says: %s\n", sd, buffer);

                    send(sd, "Message received", 16, 0);
                }
            }
        }
    }

}
