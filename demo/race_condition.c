/*
 * race_condition.c
 *
 * Operating Systems Lab Demonstration:
 * Race Condition and Data Corruption
 *
 * Two POSIX threads increment the same shared counter without any
 * synchronization. The operation counter++ is intentionally expanded into
 * read, modify, and write steps so the scheduler has more opportunity to
 * interleave both threads inside the critical section.
 *
 * Compile:
 *   gcc race_condition.c -o race_condition -lpthread
 */

#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define THREAD_COUNT 2
#define INCREMENTS_PER_THREAD 1000000L

static volatile long long shared_counter = 0;

typedef struct {
    int thread_id;
    long increments;
} thread_args_t;

static void fail_with_error(const char *message, int error_code) {
    fprintf(stderr, "ERROR: %s: %s\n", message, strerror(error_code));
    exit(EXIT_FAILURE);
}

static double elapsed_seconds(struct timespec start, struct timespec end) {
    double seconds = (double)(end.tv_sec - start.tv_sec);
    double nanoseconds = (double)(end.tv_nsec - start.tv_nsec) / 1000000000.0;
    return seconds + nanoseconds;
}

static void *unsafe_increment_worker(void *arg) {
    thread_args_t *thread_args = (thread_args_t *)arg;

    for (long i = 0; i < thread_args->increments; i++) {
        /*
         * This is the critical section, but it is deliberately unprotected.
         *
         * The three steps below model what a non-atomic counter++ operation
         * effectively does:
         *   1. LOAD  shared_counter into a register-like local variable
         *   2. ADD   one to the local value
         *   3. STORE the value back to shared memory
         *
         * If another thread runs between LOAD and STORE, one increment can
         * overwrite another. That is the lost update problem.
         */
        long long local_copy = shared_counter;

        if ((i % 128) == 0) {
            sched_yield();
        }

        local_copy = local_copy + 1;

        if ((i % 257) == 0) {
            usleep(1);
        }

        shared_counter = local_copy;
    }

    printf("[Thread_%d] completed %ld unsafe increments\n",
           thread_args->thread_id,
           thread_args->increments);

    return NULL;
}

int main(void) {
    pthread_t threads[THREAD_COUNT];
    thread_args_t thread_args[THREAD_COUNT];
    struct timespec start_time;
    struct timespec end_time;

    long long expected_counter = THREAD_COUNT * INCREMENTS_PER_THREAD;

    printf("====================================\n");
    printf("RACE CONDITION DEMONSTRATION\n");
    printf("====================================\n\n");
    printf("Threads          : %d\n", THREAD_COUNT);
    printf("Increments/Thread: %ld\n", INCREMENTS_PER_THREAD);
    printf("Synchronization  : NONE\n");
    printf("Interleaving     : sched_yield() + usleep()\n\n");

    if (clock_gettime(CLOCK_MONOTONIC, &start_time) != 0) {
        perror("clock_gettime");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < THREAD_COUNT; i++) {
        thread_args[i].thread_id = i + 1;
        thread_args[i].increments = INCREMENTS_PER_THREAD;

        int result = pthread_create(&threads[i], NULL, unsafe_increment_worker, &thread_args[i]);
        if (result != 0) {
            fail_with_error("pthread_create failed", result);
        }
    }

    for (int i = 0; i < THREAD_COUNT; i++) {
        int result = pthread_join(threads[i], NULL);
        if (result != 0) {
            fail_with_error("pthread_join failed", result);
        }
    }

    if (clock_gettime(CLOCK_MONOTONIC, &end_time) != 0) {
        perror("clock_gettime");
        return EXIT_FAILURE;
    }

    long long actual_counter = shared_counter;
    long long data_lost = expected_counter - actual_counter;
    double runtime = elapsed_seconds(start_time, end_time);

    printf("\n------------------------------------\n");
    printf("RESULT\n");
    printf("------------------------------------\n");
    printf("Expected Counter : %lld\n", expected_counter);
    printf("Actual Counter   : %lld\n", actual_counter);
    printf("Data Lost        : %lld\n\n", data_lost);
    printf("Execution Time   : %.6f sec\n", runtime);
    printf("Data Integrity   : %s\n\n", data_lost == 0 ? "PASSED" : "FAILED");
    printf("STATUS : %s\n", data_lost == 0 ? "NO CORRUPTION OBSERVED" : "DATA CORRUPTED");

    return EXIT_SUCCESS;
}
