/*
 * mutex_fix.c
 *
 * Operating Systems Lab Demonstration:
 * Mutex Synchronization and Critical Section Protection
 *
 * This program uses the same shared counter workload as race_condition.c,
 * but protects the critical section with a pthread mutex. The result should
 * be correct because only one thread may execute counter++ at a time.
 *
 * Compile:
 *   gcc mutex_fix.c -o mutex_fix -lpthread
 */

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define THREAD_COUNT 2
#define INCREMENTS_PER_THREAD 1000000L

static long long shared_counter = 0;
static pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

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

static void *safe_increment_worker(void *arg) {
    thread_args_t *thread_args = (thread_args_t *)arg;

    for (long i = 0; i < thread_args->increments; i++) {
        /*
         * Critical Section:
         * The shared counter is read, modified, and written while the mutex
         * is held. This enforces mutual exclusion and prevents lost updates.
         */
        int lock_result = pthread_mutex_lock(&counter_mutex);
        if (lock_result != 0) {
            fail_with_error("pthread_mutex_lock failed", lock_result);
        }

        shared_counter++;

        int unlock_result = pthread_mutex_unlock(&counter_mutex);
        if (unlock_result != 0) {
            fail_with_error("pthread_mutex_unlock failed", unlock_result);
        }

        /*
         * This yield is outside the critical section. Threads may interleave
         * freely, but they cannot corrupt shared_counter.
         */
        if ((i % 250000) == 0) {
            usleep(1);
        }
    }

    printf("[Thread_%d] completed %ld synchronized increments\n",
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
    printf("MUTEX SYNCHRONIZATION\n");
    printf("====================================\n\n");
    printf("Threads          : %d\n", THREAD_COUNT);
    printf("Increments/Thread: %ld\n", INCREMENTS_PER_THREAD);
    printf("Synchronization  : pthread_mutex_t\n");
    printf("Critical Section : shared_counter++\n\n");

    if (clock_gettime(CLOCK_MONOTONIC, &start_time) != 0) {
        perror("clock_gettime");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < THREAD_COUNT; i++) {
        thread_args[i].thread_id = i + 1;
        thread_args[i].increments = INCREMENTS_PER_THREAD;

        int result = pthread_create(&threads[i], NULL, safe_increment_worker, &thread_args[i]);
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

    int destroy_result = pthread_mutex_destroy(&counter_mutex);
    if (destroy_result != 0) {
        fail_with_error("pthread_mutex_destroy failed", destroy_result);
    }

    long long actual_counter = shared_counter;
    double runtime = elapsed_seconds(start_time, end_time);

    printf("\n------------------------------------\n");
    printf("RESULT\n");
    printf("------------------------------------\n");
    printf("Expected Counter : %lld\n", expected_counter);
    printf("Actual Counter   : %lld\n\n", actual_counter);
    printf("Execution Time   : %.6f sec\n", runtime);
    printf("Data Integrity   : %s\n\n", actual_counter == expected_counter ? "PASSED" : "FAILED");
    printf("STATUS : %s\n", actual_counter == expected_counter ? "SAFE" : "UNEXPECTED CORRUPTION");

    return EXIT_SUCCESS;
}
