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
void add_process(Scheduler *scheduler, Process process)
{
    PriorityQueue *queue = &scheduler->queues[process.priority];
    queue->processes[queue->count++] = process;
}

// Executa um processo (utilizando fork)
void execute_process(Process *process)
{
    process->pid = fork();

    if (process->pid == 0)
    {
        printf("Processo %d iniciado: %s\n", process->id, process->executable);
        execl(process->executable, process->executable, NULL);
        perror("Erro ao executar o processo");
        exit(EXIT_FAILURE);
    }
}

// Função principal do escalonador
void execute_scheduler(Scheduler *scheduler, const char *input_file)
{
    FILE *file = fopen(input_file, "r");
    if (!file)
    {
        perror("Erro ao abrir o arquivo");
        exit(EXIT_FAILURE);
    }

    char line[256];
    while (fgets(line, sizeof(line), file))
    {
        Process process;
        sscanf(line, "%d %s %d %d", &process.id, process.executable,
               &process.start_time, &process.priority);
        process.pid = -1;
        add_process(scheduler, process);
    }
    fclose(file);

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
                if (process->start_time > 0)
                {
                    sleep(1);
                    process->start_time--;
                    continue;
                }

                if (process->pid == -1)
                {
                    execute_process(process);
                }

                // Executar pelo quantum
                sleep(scheduler->quantum);
                kill(process->pid, SIGSTOP);

                // Verifica se o processo ainda está rodando
                int status;
                if (waitpid(process->pid, &status, WNOHANG) == process->pid)
                {
                    printf("Processo %d concluído.\n", process->id);
                    process->pid = 0; // Marca como concluído
                }
                else
                {
                    kill(process->pid, SIGCONT); // Retoma execução
                    running = 1;                 // Continua o escalonamento
                }
            }
        }
    }

    printf("Todos os processos foram concluídos.\n");
}
