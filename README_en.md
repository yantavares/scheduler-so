# Unix Static Priority Multicore Scheduler

```
                            _____  .__ ___.
  __________               /  _  \ |  |\_ |__ _____
 /  ___/  _ \    ______   /  /_\  \|  | | __ \\__  \
 \___ (  <_> )  /_____/  /    |    \  |_| \_\ \/ __ \_
/____  >____/            \____|__  /____/___  (____  /
     \/                          \/         \/     \/

```

This program implements a scheduler that manages processes in a Unix environment using the Round Robin policy with up to 4 priority queues (0, 1, 2, 3) and a specific quantum. It simulates a multicore environment, scheduling processes based on their priorities and start times.

---

## Compilation

### Option 1: Using the Makefile

```bash
make
```

This will compile the program and generate the `escalona` executable, along with compiling all test processes in the `process` directory.

### Option 2: Manual Compilation

```bash
gcc -o escalona scheduler.c -lpthread
```

## Execution

```bash
./escalona <number_of_cores> <quantum> <input_file>
```

Example:

```bash
./escalona 4 2 input.txt
```

Two test input files are already available in the scheduler's directory.

### **Input File Format:**

The input file must follow the format:

```
<id> <executable> <start_time> <priority>
```

Example:

```
1 test20 0 2
2 test10 0 0
3 test30 20 0
4 test10 15 1
```

---

## Features

- Manages processes in 4 priority queues.
- Supports round-robin for each queue.
- Uses semaphores to manage core access.
- Monitors processes via pipes for interprocess communication.
- Generates a final report with turnaround time and execution details for each process.

---

### Final Report

The program outputs:

- Execution order of the processes.
- Turnaround time for each process.
- Average turnaround time.

---

## Notes

- The functionality for multiple cores is not yet implemented. The scheduling logic is in place, but parallel execution is still under development. (Work in progress)
