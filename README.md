# Race Condition & Thread Synchronization — Interactive Demo

Interactive web presentation plus small C terminal demos that illustrate race conditions, critical sections, mutexes, and semaphores.

What’s included
- index.html, styles.css, script.js — interactive presentation/visualization
- demo/ — C programs and demo README:
  - race_condition.c — unsafe counter (lost updates)
  - mutex_fix.c — same workload with pthread mutex
  - semaphore_pc.c — bounded-buffer producer/consumer with POSIX semaphores
  - scheduler_simulation.c — educational scheduler trace

Requirements
- Browser (to open index.html)
- Linux / POSIX environment for C demos:
  - gcc
  - pthreads (link with -lpthread or -pthread)
  - POSIX semaphores (header <semaphore.h>)

Build (from repo root)
```bash
cd demo
gcc race_condition.c -o race_condition -lpthread
gcc mutex_fix.c -o mutex_fix -lpthread
gcc semaphore_pc.c -o semaphore_pc -lpthread
gcc scheduler_simulation.c -o scheduler_simulation
```
(If needed, use `-pthread` instead of `-lpthread`.)

Run
```bash
# Presentation
# open index.html in a browser or serve it:
python3 -m http.server 8000
# then open http://localhost:8000

# Demos (from demo/)
./scheduler_simulation
./race_condition
./mutex_fix
./semaphore_pc
```

Suggested demo flow
1. Run `scheduler_simulation` to explain interleaving/context switch.
2. Run `race_condition` to show data corruption.
3. Run `mutex_fix` to show the mutex solution.
4. Run `semaphore_pc` to show producer-consumer coordination.
