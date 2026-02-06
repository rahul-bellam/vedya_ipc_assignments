#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>

#define PORT 8080
#define MAX_CLIENTS 5
#define BUFFER_SIZE 1024

static int client_socket[MAX_CLIENTS];
static pthread_mutex_t clients_lock = PTHREAD_MUTEX_INITIALIZER;

/* Remove a client fd from the global list */
static void remove_client(int sd) {
    pthread_mutex_lock(&clients_lock);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_socket[i] == sd) {
            client_socket[i] = 0;
            break;
        }
    }
    pthread_mutex_unlock(&clients_lock);
}

/* Thread function to handle one client */
static void *client_handler(void *arg) {
    int sd = *(int *)arg;
    free(arg);

    char buffer[BUFFER_SIZE + 1]; // +1 for '\0'

    while (1) {
        int valread = recv(sd, buffer, BUFFER_SIZE, 0);

        if (valread == 0) {
            // Peer closed the connection
            printf("Client on socket %d disconnected\n", sd);
            break;
        } else if (valread < 0) {
            // Error
            perror("recv");
            break;
        }

        buffer[valread] = '\0';
        printf("Client %d says: %s\n", sd, buffer);

        const char *ack = "Message received";
        if (send(sd, ack, strlen(ack), 0) < 0) {
            perror("send");
            break;
        }
    }

    close(sd);
    remove_client(sd);

    return NULL;
}

int main() {
    int server_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    // Initialize client sockets
    memset(client_socket, 0, sizeof(client_socket));

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Recommended: allow quick restart
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt(SO_REUSEADDR)");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        int new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (new_socket < 0) {
            perror("accept");
            continue;
        }

        printf("New client connected: socket fd %d\n", new_socket);

        // Try to store the client in an available slot
        int stored = 0;

        pthread_mutex_lock(&clients_lock);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_socket[i] == 0) {
                client_socket[i] = new_socket;
                stored = 1;
                break;
            }
        }
        pthread_mutex_unlock(&clients_lock);

        if (!stored) {
            // Server full: notify and close
            const char *busy = "Server busy. Try again later.\n";
            send(new_socket, busy, strlen(busy), 0);
            close(new_socket);
            continue;
        }

        // Launch a thread to handle this client
        pthread_t tid;
        int *pclient = malloc(sizeof(int));
        if (!pclient) {
            perror("malloc");
            close(new_socket);
            remove_client(new_socket);
            continue;
        }
        *pclient = new_socket;

        if (pthread_create(&tid, NULL, client_handler, pclient) != 0) {
            perror("pthread_create");
            close(new_socket);
            remove_client(new_socket);
            free(pclient);
            continue;
        }

        pthread_detach(tid);
    }

    close(server_fd);
    return 0;
}
