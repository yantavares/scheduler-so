#include "scheduler.h"

// Inicializa o escalonador
void init_scheduler(Scheduler *scheduler, int quantum)
{
    scheduler->quantum = quantum;
    for (int i = 0; i < MAX_PRIORITY; i++)
    {
        scheduler->queues[i].count = 0;
    }
}

// Adiciona um processo à fila de prioridade correspondente
void add_process(Scheduler *scheduler, Process *process)
{
    PriorityQueue *queue = &scheduler->queues[process->priority];
    queue->processes[queue->count++] = *process;
}

// Executa um processo e configura o pipe para comunicação
void execute_process(Process *process)
{
    if (pipe(process->pipe_fd) == -1)
    {
        perror("Erro ao criar pipe");
        exit(EXIT_FAILURE);
    }

    process->pid = fork();
    if (process->pid == 0)
    {
        // Código do processo filho
        close(process->pipe_fd[0]); // Fecha a leitura no filho
        dup2(process->pipe_fd[1], STDOUT_FILENO);
        close(process->pipe_fd[1]);

        execl(process->executable, process->executable, NULL);
        perror("Erro ao executar o processo");
        exit(EXIT_FAILURE);
    }

    // Código do processo pai
    close(process->pipe_fd[1]); // Fecha a escrita no pai
}

void execute_scheduler(Scheduler *scheduler, const char *input_file)
{
    signal(SIGSTOP, SIG_IGN);
    signal(SIGCONT, SIG_IGN);

    // Tempo base do escalonador
    time_t start_time_global = time(NULL);

    // Lendo o arquivo de entrada
    FILE *file = fopen(input_file, "r");
    if (!file)
    {
        perror("Erro ao abrir o arquivo");
        exit(EXIT_FAILURE);
    }

    printf("Lendo arquivo de entrada:\n");
    char line[256];
    while (fgets(line, sizeof(line), file))
    {
        Process process;
        sscanf(line, "%d %s %d %d", &process.id, process.executable,
               &process.start_time, &process.priority);
        process.pid = -1;
        process.status = READY;
        process.start_time_original = process.start_time;
        printf("Processo %d - Executável: %s, Início: %d, Prioridade: %d\n",
               process.id, process.executable, process.start_time, process.priority);
        add_process(scheduler, &process);
    }
    fclose(file);

    printf("\nIniciando o escalonador...\n");
    int running = 1;

    while (running)
    {
        running = 0;

        for (int priority = 0; priority < MAX_PRIORITY; priority++)
        {
            PriorityQueue *queue = &scheduler->queues[priority];

            for (int i = 0; i < queue->count; i++)
            {
                Process *process = &queue->processes[i];

                if (process->status == FINISHED)
                    continue;

                if (process->start_time > 0)
                {
                    printf("Processo %d aguardando início (restante: %d s)\n",
                           process->id, process->start_time);
                    sleep(1);
                    process->start_time--;
                    continue;
                }

                if (process->status == READY && process->start_time == 0)
                {
                    execute_process(process);
                    process->status = RUNNING;
                    printf("Processo %d iniciado: PID %d\n", process->id, process->pid);
                }

                if (process->status == RUNNING || process->status == SUSPENDED)
                {
                    printf("Enviando SIGCONT para Processo %d (PID %d)\n", process->id, process->pid);
                    kill(process->pid, SIGCONT);
                    sleep(scheduler->quantum);

                    int status;
                    if (waitpid(process->pid, &status, WNOHANG) == process->pid)
                    {
                        printf("Processo %d concluído.\n", process->id);
                        process->status = FINISHED;
                        close(process->pipe_fd[0]);
                        process->end_time = time(NULL);
                    }
                    else
                    {
                        printf("Enviando SIGSTOP para Processo %d (PID %d)\n", process->id, process->pid);
                        kill(process->pid, SIGSTOP);
                        process->status = SUSPENDED;
                        running = 1;
                    }
                }
            }
        }
    }

    printf("\nTodos os processos foram concluídos.\n");

    // Relatório Final
    printf("\nRelatório Final:\n");
    int total_turnaround = 0;
    int total_processes = 0;

    for (int priority = 0; priority < MAX_PRIORITY; priority++)
    {
        PriorityQueue *queue = &scheduler->queues[priority];
        for (int i = 0; i < queue->count; i++)
        {
            Process *process = &queue->processes[i];
            if (process->status == FINISHED)
            {
                int turnaround = (int)(process->end_time - start_time_global - process->start_time_original);
                printf("Processo %d: Turnaround = %d segundos\n", process->id, turnaround);
                total_turnaround += turnaround;
                total_processes++;
            }
        }
    }

    if (total_processes > 0)
    {
        printf("Tempo médio de turnaround: %.2f segundos\n",
               (float)total_turnaround / total_processes);
    }
}
