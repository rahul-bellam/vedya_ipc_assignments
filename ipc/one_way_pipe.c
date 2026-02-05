#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main() {
    int fd[2];
    pipe(fd);          // fd[0] = read, fd[1] = write

    if (fork() == 0) { // Child
        close(fd[1]); // Close write end
        char buf[100];
        read(fd[0], buf, sizeof(buf));
        printf("Child received: %s\n", buf);
        close(fd[0]);
    } else {           // Parent
        close(fd[0]); // Close read end
        char msg[] = "Hello from parent";
        write(fd[1], msg, strlen(msg) + 1);
        close(fd[1]);
    }

    return 0;
}