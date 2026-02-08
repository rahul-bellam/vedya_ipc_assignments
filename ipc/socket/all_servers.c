#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <termios.h>

#define TCP_PORT 9000
#define UDP_PORT 9001
#define SERIAL_DEV "/dev/ttyS0"
#define BUF_SIZE 1024

int setup_serial()
{
    int fd = open(SERIAL_DEV, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        perror("serial open");
        return -1;
    }

    struct termios tty;
    tcgetattr(fd, &tty);
    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);
    tty.c_cflag |= (CLOCAL | CREAD);
    tcsetattr(fd, TCSANOW, &tty);

    return fd;
}

int main()
{
    int tcp_sock, udp_sock, serial_fd;
    int max_fd;
    fd_set readfds;

    struct sockaddr_in tcp_addr, udp_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUF_SIZE];

    // tcp setup
    tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
    tcp_addr.sin_family = AF_INET;
    tcp_addr.sin_addr.s_addr = INADDR_ANY;
    tcp_addr.sin_port = htons(TCP_PORT);
    bind(tcp_sock, (struct sockaddr*)&tcp_addr, sizeof(tcp_addr));
    listen(tcp_sock, 5);

    // udp setup

    udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_addr.s_addr = INADDR_ANY;
    udp_addr.sin_port = htons(UDP_PORT);
    bind(udp_sock, (struct sockaddr*)&udp_addr, sizeof(udp_addr));

    // serial

    serial_fd = setup_serial();

    max_fd = tcp_sock;
    if (udp_sock > max_fd) max_fd = udp_sock;
    if (serial_fd > max_fd) max_fd = serial_fd;

    printf("Server running: TCP(%d), UDP(%d), SERIAL(%s)\n",
           TCP_PORT, UDP_PORT, SERIAL_DEV);

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(tcp_sock, &readfds);
        FD_SET(udp_sock, &readfds);
        if (serial_fd > 0)
            FD_SET(serial_fd, &readfds);

        select(max_fd + 1, &readfds, NULL, NULL, NULL);

       // tcp client 

        if (FD_ISSET(tcp_sock, &readfds)) {
            int client = accept(tcp_sock,
                                (struct sockaddr*)&client_addr,
                                &addr_len);
            read(client, buffer, BUF_SIZE);
            printf("[TCP] %s\n", buffer);
            write(client, "TCP ACK\n", 8);
            close(client);
        }

        // udp client

        if (FD_ISSET(udp_sock, &readfds)) {
            recvfrom(udp_sock, buffer, BUF_SIZE, 0,
                     (struct sockaddr*)&client_addr, &addr_len);
            printf("[UDP] %s\n", buffer);
            sendto(udp_sock, "UDP ACK\n", 8, 0,
                   (struct sockaddr*)&client_addr, addr_len);
        }

        // serial connection client
        
        if (serial_fd > 0 && FD_ISSET(serial_fd, &readfds)) {
            int n = read(serial_fd, buffer, BUF_SIZE);
            if (n > 0) {
                buffer[n] = 0;
                printf("[SERIAL] %s\n", buffer);
            }
        }
    }
}
