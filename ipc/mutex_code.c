//for the code of alternating counter using mutexes
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdatomic.h>
pthread_t t1, t2, t3, t4;

atomic_int signal4 = 0;
atomic_int signal3 = 0;

void* T1_function(void* arg) {
    int count = 0;
    int limit = *(int*)arg; 

    printf("T1: Started with limit factor %d\n", limit);
    while ((count / 20) < limit) {
        pthread_testcancel();
        sleep(1);
        count++;
        printf("T1: Counter = %d\n", (count%20));

        if (count == 10) {
            printf("--- Turning on T4 from T1 ---\n");
            signal4 = 1;
        }
    }

    printf("T1: Limit reached. Canceling other threads.\n");
    pthread_cancel(t3);
    pthread_cancel(t4);
    return NULL;
}

void* T4_function(void* args) {
    while (1) {
        pthread_testcancel();
        if (signal4) {
            printf("T4 is ON\n");
            for (int i = 1; i <= 5; i++) {
                sleep(1);
                printf("T4 counter at = %d\n", i);
                if (i == 5) {
                    printf("--- Turning on T3 from T4 ---\n");
                    signal3 = 1;
                }
            }
        }
    }
}

void* T3_function(void* args) {
    while (1) {
        pthread_testcancel();
        if (signal3) {
            printf("T3 is ON\n");
            for (int i = 1; i <= 5; i++) {
                sleep(1); 
                printf("T3 counter at = %d\n", i);
            }
        }
    }
}

int main() {
    printf("Enter the limit you want it to run: ");
    int lim;
    scanf("%d", &lim);
    pthread_create(&t1, NULL, T1_function, &lim);
    pthread_create(&t3, NULL, T3_function, NULL);
    pthread_create(&t4, NULL, T4_function, NULL);
    pthread_join(t1, NULL);
    pthread_join(t3, NULL);
    pthread_join(t4, NULL);
    printf("Main: Execution Finished.\n");
    return 0;
} 