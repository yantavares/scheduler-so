#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#define MAX_PROCESSES 100
#define MAX_PRIORITY 4

typedef enum
{
    READY,
    RUNNING,
    SUSPENDED,
    FINISHED
} ProcessStatus;

typedef struct
{
    int id;
    char executable[256];
    int start_time;
    int priority;
    pid_t pid;
    int pipe_fd[2];
    ProcessStatus status;
    time_t start_time_original; // Tempo de início original
    time_t end_time;            // Tempo de término
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
void add_process(Scheduler *scheduler, Process *process);
void execute_scheduler(Scheduler *scheduler, const char *input_file);
void execute_process(Process *process);

#endif
