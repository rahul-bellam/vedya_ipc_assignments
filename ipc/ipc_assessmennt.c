#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#define MAX_MESSAGE_LENGTH 2
pthread_mutex_t mutex;
char message[MAX_MESSAGE_LENGTH] = "";
int current_thread = 1;
void* thread_function(void* arg) {
    int thread_id = *((int*)arg);
    while (1) {
        pthread_mutex_lock(&mutex);
        if (current_thread == thread_id) {
            if (thread_id == 1) {
                printf("T1: Enter message: ");
                fgets(message, MAX_MESSAGE_LENGTH, stdin);
                message[strcspn(message, "\n")] = 0; // Remove newline
            } else {
                printf("T%d: Received Message: \"%s\"\n", thread_id, message);
            }
            current_thread = (current_thread % 4) + 1; // Move to next thread
        }
        pthread_mutex_unlock(&mutex);
        usleep(100000); // Sleep for 100ms to prevent busy waiting
    }
    return NULL;
}
void signal_handler(int signum) {
    printf("signal bypassed");
}
int main() {
    signal(SIGINT, signal_handler);
    signal(SIGKILL, signal_handler);
    pthread_t threads[4];
    int thread_ids[4] = {1, 2, 3, 4};
    pthread_mutex_init(&mutex, NULL);
    for (int i = 0; i < 4; i++) {
        pthread_create(&threads[i], NULL, thread_function, &thread_ids[i]);
    }
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }
    pthread_mutex_destroy(&mutex);
    return 0;
}