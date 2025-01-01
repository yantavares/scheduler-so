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
#include <semaphore.h>

#define MAX_PROCESSES 100
#define MAX_PRIORITY 4

typedef enum
{
    READY,
    RUNNING,
    SUSPENDED,
    FINISHED
} ProcessStatus;

typedef struct Process
{
    int id;
    char executable[256];
    int start_time;          // Tempo de início relativo
    int start_time_original; // Tempo original de início
    int priority;
    int execution_time; // Tempo total de execução
    pid_t pid;
    int pipe_fd[2];
    ProcessStatus status;
    time_t start_execution_time; // Tempo em que a execução começou
    time_t last_resume_time;     // Último tempo em que o processo foi retomado
    time_t end_time;             // Tempo em que o processo terminou
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
    sem_t available_cores;
} Scheduler;

void init_scheduler(Scheduler *scheduler, int quantum, int num_cores);
void add_process(Scheduler *scheduler, Process *process);
void execute_scheduler(Scheduler *scheduler, const char *input_file);
void execute_process(Process *process);

#endif
