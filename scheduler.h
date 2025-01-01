#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_PROCESSES 100
#define MAX_PRIORITY 4

typedef struct
{
    int id;
    char executable[256];
    int start_time;
    int priority;
    pid_t pid;
} Process;

typedef struct
{
    Process processes[MAX_PROCESSES];
    int count;
} PriorityQueue;

typedef struct
{
    PriorityQueue queues[MAX_PRIORITY];
    int quantum;
} Scheduler;

// Funções do escalonador
void init_scheduler(Scheduler *scheduler, int quantum);
void add_process(Scheduler *scheduler, Process process);
void execute_scheduler(Scheduler *scheduler, const char *input_file);
void execute_process(Process *process);

#endif
