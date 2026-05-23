/*
 * semaphore_pc.c
 *
 * Operating Systems Lab Demonstration:
 * Producer-Consumer with POSIX Semaphores
 *
 * A bounded circular buffer is shared by one producer and one consumer.
 * Semaphores coordinate access:
 *   empty : number of empty slots
 *   full  : number of filled slots
 *   mutex : binary semaphore for mutual exclusion inside the buffer
 *
 * Compile:
 *   gcc semaphore_pc.c -o semaphore_pc -lpthread
 */

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define ITEMS_TO_PRODUCE 12

static int buffer[BUFFER_SIZE];
static int buffer_occupied[BUFFER_SIZE];
static int write_index = 0;
static int read_index = 0;

static sem_t empty_slots;
static sem_t full_slots;
static sem_t buffer_mutex;

static void fail_with_error(const char *message, int error_code) {
    fprintf(stderr, "ERROR: %s: %s\n", message, strerror(error_code));
    exit(EXIT_FAILURE);
}

static void fail_with_errno(const char *message) {
    fprintf(stderr, "ERROR: %s: %s\n", message, strerror(errno));
    exit(EXIT_FAILURE);
}

static void sleep_for_demo_timing(useconds_t microseconds) {
    usleep(microseconds);
}

static void print_buffer_state(void) {
    printf("Buffer   : [");
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (buffer_occupied[i]) {
            printf("%2d", buffer[i]);
        } else {
            printf(" -");
        }

        if (i != BUFFER_SIZE - 1) {
            printf(", ");
        }
    }
    printf("]\n");
}

static void print_semaphore_state(const char *actor) {
    int empty_value;
    int full_value;
    int mutex_value;

    sem_getvalue(&empty_slots, &empty_value);
    sem_getvalue(&full_slots, &full_value);
    sem_getvalue(&buffer_mutex, &mutex_value);

    printf("Semaphores after %-8s empty=%d full=%d mutex=%d\n",
           actor,
           empty_value,
           full_value,
           mutex_value);
}

static void *producer_thread(void *arg) {
    (void)arg;

    for (int item = 1; item <= ITEMS_TO_PRODUCE; item++) {
        int produced_value = item * 7;

        if (sem_wait(&empty_slots) != 0) {
            fail_with_errno("sem_wait(empty_slots) failed");
        }

        if (sem_wait(&buffer_mutex) != 0) {
            fail_with_errno("sem_wait(buffer_mutex) failed");
        }

        buffer[write_index] = produced_value;
        buffer_occupied[write_index] = 1;
        write_index = (write_index + 1) % BUFFER_SIZE;

        printf("\n[PRODUCER]\n");
        printf("Produced : %d\n", produced_value);
        print_buffer_state();

        if (sem_post(&buffer_mutex) != 0) {
            fail_with_errno("sem_post(buffer_mutex) failed");
        }

        if (sem_post(&full_slots) != 0) {
            fail_with_errno("sem_post(full_slots) failed");
        }

        print_semaphore_state("produce");
        sleep_for_demo_timing(140000);
    }

    return NULL;
}

static void *consumer_thread(void *arg) {
    (void)arg;

    for (int i = 0; i < ITEMS_TO_PRODUCE; i++) {
        if (sem_wait(&full_slots) != 0) {
            fail_with_errno("sem_wait(full_slots) failed");
        }

        if (sem_wait(&buffer_mutex) != 0) {
            fail_with_errno("sem_wait(buffer_mutex) failed");
        }

        int consumed_value = buffer[read_index];
        buffer_occupied[read_index] = 0;
        read_index = (read_index + 1) % BUFFER_SIZE;

        printf("\n[CONSUMER]\n");
        printf("Consumed : %d\n", consumed_value);
        print_buffer_state();

        if (sem_post(&buffer_mutex) != 0) {
            fail_with_errno("sem_post(buffer_mutex) failed");
        }

        if (sem_post(&empty_slots) != 0) {
            fail_with_errno("sem_post(empty_slots) failed");
        }

        print_semaphore_state("consume");
        sleep_for_demo_timing(220000);
    }

    return NULL;
}

int main(void) {
    pthread_t producer;
    pthread_t consumer;

    printf("====================================\n");
    printf("SEMAPHORE PRODUCER-CONSUMER DEMO\n");
    printf("====================================\n\n");
    printf("Buffer Size      : %d\n", BUFFER_SIZE);
    printf("Items Produced   : %d\n", ITEMS_TO_PRODUCE);
    printf("Semaphores       : empty, full, mutex\n\n");

    if (sem_init(&empty_slots, 0, BUFFER_SIZE) != 0) {
        fail_with_errno("sem_init(empty_slots) failed");
    }

    if (sem_init(&full_slots, 0, 0) != 0) {
        fail_with_errno("sem_init(full_slots) failed");
    }

    if (sem_init(&buffer_mutex, 0, 1) != 0) {
        fail_with_errno("sem_init(buffer_mutex) failed");
    }

    int result = pthread_create(&producer, NULL, producer_thread, NULL);
    if (result != 0) {
        fail_with_error("pthread_create(producer) failed", result);
    }

    result = pthread_create(&consumer, NULL, consumer_thread, NULL);
    if (result != 0) {
        fail_with_error("pthread_create(consumer) failed", result);
    }

    result = pthread_join(producer, NULL);
    if (result != 0) {
        fail_with_error("pthread_join(producer) failed", result);
    }

    result = pthread_join(consumer, NULL);
    if (result != 0) {
        fail_with_error("pthread_join(consumer) failed", result);
    }

    sem_destroy(&empty_slots);
    sem_destroy(&full_slots);
    sem_destroy(&buffer_mutex);

    printf("\n====================================\n");
    printf("STATUS : PRODUCER-CONSUMER COORDINATED\n");
    printf("====================================\n");

    return EXIT_SUCCESS;
}
