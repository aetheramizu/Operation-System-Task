/*
 * scheduler_simulation.c
 *
 * Operating Systems Storytelling Demonstration:
 * Educational Context-Switch Trace
 *
 * This program is not a real scheduler. It is a terminal visualization that
 * mirrors the website's race condition sequence: Thread A starts incrementing
 * a shared counter, the scheduler switches execution to Thread B, then Thread
 * A resumes and overwrites the same value.
 *
 * Compile:
 *   gcc scheduler_simulation.c -o scheduler_simulation
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void pause_step(void) {
    usleep(650000);
}

static void log_step(const char *actor, const char *message) {
    printf("%-14s: %s\n", actor, message);
    fflush(stdout);
    pause_step();
}

int main(void) {
    int counter = 100;
    int register_a = 0;
    int register_b = 0;

    printf("====================================\n");
    printf("CONTEXT SWITCH TRACE SIMULATION\n");
    printf("====================================\n\n");
    printf("Initial shared counter = %d\n\n", counter);
    pause_step();

    log_step("Thread A", "LOAD counter into R1");
    register_a = counter;
    log_step("Thread A", "ADD 1 to R1");
    register_a = register_a + 1;

    log_step("[SCHEDULER]", "Timer interrupt: context switch");

    log_step("Thread B", "LOAD counter into R2");
    register_b = counter;
    log_step("Thread B", "ADD 1 to R2");
    register_b = register_b + 1;
    log_step("Thread B", "STORE R2 back to shared counter");
    counter = register_b;
    printf("               counter is now %d\n", counter);
    fflush(stdout);
    pause_step();

    log_step("[SCHEDULER]", "Resume Thread A");

    log_step("Thread A", "STORE old R1 back to shared counter");
    counter = register_a;
    printf("               counter is now %d\n\n", counter);
    fflush(stdout);
    pause_step();

    printf("------------------------------------\n");
    printf("EXPECTED RESULT : 102\n");
    printf("ACTUAL RESULT   : %d\n", counter);
    printf("STATUS          : LOST UPDATE CAUSED BY CONTEXT SWITCH\n");
    printf("------------------------------------\n");

    return EXIT_SUCCESS;
}
