# Operating Systems Technical Demo

This folder contains real C demonstration programs for the interactive presentation:

**Race Condition & Thread Synchronization**

The demos match the same concepts shown in the website:

- concurrency
- thread execution
- context switching
- race condition
- critical section
- mutex synchronization
- semaphore coordination
- bounded buffer producer-consumer
- performance and correctness tradeoff

These programs are designed for terminal demonstrations, report screenshots, and video recording.

## Requirements

Linux or POSIX-compatible environment:

- `gcc`
- `pthread`
- POSIX semaphores

## Build Commands

```bash
gcc race_condition.c -o race_condition -lpthread
gcc mutex_fix.c -o mutex_fix -lpthread
gcc semaphore_pc.c -o semaphore_pc -lpthread
gcc scheduler_simulation.c -o scheduler_simulation
```

On some Linux distributions, `-pthread` may be preferred:

```bash
gcc race_condition.c -o race_condition -pthread
gcc mutex_fix.c -o mutex_fix -pthread
gcc semaphore_pc.c -o semaphore_pc -pthread
```

## Run Commands

```bash
./race_condition
./mutex_fix
./semaphore_pc
./scheduler_simulation
```

## Demo 1: `race_condition.c`

Demonstrates an unsafe shared counter.

Two threads increment the same global counter one million times each. The program intentionally uses `sched_yield()` and `usleep()` to increase interleaving, making the lost update problem easier to observe.

Learning outcome:

- `counter++` is not atomic.
- Shared memory without synchronization can corrupt data.
- The actual result may be lower than the expected result.

## Demo 2: `mutex_fix.c`

Demonstrates a mutex-protected critical section.

The workload is the same as `race_condition.c`, but `shared_counter++` is protected by:

```c
pthread_mutex_lock(&counter_mutex);
shared_counter++;
pthread_mutex_unlock(&counter_mutex);
```

Learning outcome:

- Mutex provides mutual exclusion.
- Only one thread can enter the critical section at a time.
- Correctness improves, but synchronization adds overhead.

## Demo 3: `semaphore_pc.c`

Demonstrates the bounded buffer producer-consumer problem.

The program uses:

- `empty` semaphore for available empty slots
- `full` semaphore for available filled slots
- `mutex` semaphore for exclusive buffer access

Learning outcome:

- Semaphores coordinate access between producer and consumer.
- A circular buffer needs both counting semaphores and mutual exclusion.
- Producer must wait when the buffer is full.
- Consumer must wait when the buffer is empty.

## Demo 4: `scheduler_simulation.c`

Creates a cinematic terminal trace of a context switch.

This is not a real CPU scheduler. It is an educational log sequence that matches the website visualization:

1. Thread A loads counter.
2. Thread A increments local register.
3. Scheduler interrupts.
4. Thread B loads the old counter.
5. Thread B stores `101`.
6. Thread A resumes and also stores `101`.

Learning outcome:

- Context switching can expose race conditions.
- The lost update problem can happen even when both threads execute logically correct code.

## Suggested Presentation Flow

1. Run `scheduler_simulation` to explain the story.
2. Run `race_condition` to show real data corruption.
3. Run `mutex_fix` to show the safe version.
4. Run `semaphore_pc` to show coordination beyond a simple mutex.

## Sample Outputs

Sample terminal outputs are stored in:

```text
outputs/race_output.txt
outputs/mutex_output.txt
outputs/semaphore_output.txt
```

Actual output may vary because thread scheduling is nondeterministic.
