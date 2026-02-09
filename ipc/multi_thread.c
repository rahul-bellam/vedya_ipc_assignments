#define _GNU_SOURCE
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <time.h>

static volatile int keep_running = 1;

typedef struct {
    int thread_index;
    int pin;            // 1 = pin threads, 0 = don't pin
    int cpu_count;
} thread_arg_t;

static long get_tid(void) {
    return (long)syscall(SYS_gettid); // Linux thread id
}

static void pin_to_cpu(int cpu) {
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(cpu, &set);
    int rc = sched_setaffinity(0, sizeof(set), &set);
    if (rc != 0) {
        perror("sched_setaffinity");
    }
}

static void* worker(void* arg) {
    thread_arg_t* a = (thread_arg_t*)arg;

    long tid = get_tid();
    int cpu = -1;

    if (a->pin) {
        cpu = a->thread_index % a->cpu_count;
        pin_to_cpu(cpu);
    }

    // Show PID/TID so you can inspect /proc
    pid_t pid = getpid();
    int running_cpu = sched_getcpu();
    printf("[thread %d] PID=%d TID=%ld pinned_cpu=%d running_cpu=%d\n",
           a->thread_index, pid, tid, cpu, running_cpu);
    fflush(stdout);

    // CPU-bound loop: keep CPU busy
    // Do some computation to avoid compiler optimizing it away
    uint64_t x = (uint64_t)(tid * 1469598103934665603ULL);

    while (keep_running) {
        // simple mixing operations
        x ^= x << 13;
        x ^= x >> 7;
        x ^= x << 17;

        // occasional small pause to reduce runaway heat (optional)
        // comment out for full burn
        if ((x & 0xFFFFFF) == 0) {
            // yield CPU briefly
            sched_yield();
        }
    }

    // print final value so compiler can't remove loop
    printf("[thread %d] TID=%ld exiting, x=%llu\n",
           a->thread_index, tid, (unsigned long long)x);
    return NULL;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr,
            "Usage: %s <num_threads> [seconds_to_run] [pin(0|1)]\n"
            "Example: %s 8 20 1\n", argv[0], argv[0]);
        return 1;
    }

    int nthreads = atoi(argv[1]);
    int seconds = (argc >= 3) ? atoi(argv[2]) : 15;
    int pin = (argc >= 4) ? atoi(argv[3]) : 1;

    if (nthreads <= 0) {
        fprintf(stderr, "num_threads must be > 0\n");
        return 1;
    }

    int cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
    if (cpu_count <= 0) cpu_count = 1;

    printf("Main PID=%d, online_cpus=%d, threads=%d, seconds=%d, pin=%d\n",
           getpid(), cpu_count, nthreads, seconds, pin);

    pthread_t* threads = (pthread_t*)calloc(nthreads, sizeof(pthread_t));
    thread_arg_t* args = (thread_arg_t*)calloc(nthreads, sizeof(thread_arg_t));
    if (!threads || !args) {
        perror("calloc");
        return 1;
    }

    for (int i = 0; i < nthreads; i++) {
        args[i].thread_index = i;
        args[i].pin = pin;
        args[i].cpu_count = cpu_count;

        int rc = pthread_create(&threads[i], NULL, worker, &args[i]);
        if (rc != 0) {
            fprintf(stderr, "pthread_create failed for thread %d (rc=%d)\n", i, rc);
            return 1;
        }
    }

    // Let them run for a while
    sleep(seconds);

    keep_running = 0;

    for (int i = 0; i < nthreads; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    free(args);

    printf("Done.\n");
    return 0;
}
