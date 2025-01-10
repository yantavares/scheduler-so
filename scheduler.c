#include "scheduler.h"

// Inicializa o escalonador
void init_scheduler(Scheduler *scheduler, int quantum, int num_cores)
{
    scheduler->quantum = quantum;
    for (int i = 0; i < MAX_PRIORITY; i++)
    {
        scheduler->queues[i].count = 0;
    }
    sem_init(&scheduler->available_cores, 0, num_cores); // Inicializa semáforo para cores
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
    close(process->pipe_fd[1]);                 // Fecha a escrita no pai
    process->start_execution_time = time(NULL); // Registra o início da execução
}

void execute_scheduler(Scheduler *scheduler, const char *input_file)
{
    signal(SIGSTOP, SIG_IGN);
    signal(SIGCONT, SIG_IGN);

    time_t start_time_global = time(NULL);
    time_t current_time = start_time_global;

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
        process.execution_time = 0;
        printf("Processo %d - Executável: %s, Início: %d, Prioridade: %d\n",
               process.id, process.executable, process.start_time, process.priority);
        add_process(scheduler, &process);
    }
    fclose(file);

    printf("\nIniciando o escalonador...\n");
    int running = 1;

    while (running)
    {
        running = 0;                      // Assume que todos os processos terminaram
        time_t current_time = time(NULL); // Atualiza o tempo atual

        for (int priority = 0; priority < MAX_PRIORITY; priority++)
        {
            PriorityQueue *queue = &scheduler->queues[priority];

            for (int i = 0; i < queue->count; i++)
            {
                Process *process = &queue->processes[i];

                if (process->status == FINISHED)
                {
                    continue;
                }

                running = 1;

                // Verifica se o processo já está elegível com base no tempo global
                if (process->start_time > (int)(current_time - start_time_global))
                {
                    continue;
                }
                else if (process->arrived == 0)
                {
                    process->arrived = 1;
                    printf("Processo %d chegou.\n", process->id);
                }

                // Iniciar o processo
                if (process->status == READY)
                {
                    if (sem_trywait(&scheduler->available_cores) == 0)
                    {
                        printf("Semáforo obtido para Processo %d\n", process->id);
                        execute_process(process);
                        process->status = RUNNING;
                        printf("Processo %d iniciado: PID %d\n", process->id, process->pid);
                    }
                    else
                    {
                        // Semáforo não disponível, verificar outros processos
                        continue;
                    }
                }

                // Retomar ou verificar um processo em execução
                if (process->status == RUNNING || process->status == SUSPENDED)
                {
                    printf("Enviando SIGCONT para Processo %d (PID %d)\n", process->id, process->pid);
                    kill(process->pid, SIGCONT);
                    sleep(scheduler->quantum);

                    int status;
                    pid_t result = waitpid(process->pid, &status, WNOHANG);

                    if (result > 0)
                    {
                        if (WIFEXITED(status))
                        {
                            printf("Processo %d finalizou normalmente.\n", process->id);
                        }
                        else
                        {
                            printf("Processo %d terminou com erro ou sinal.\n", process->id);
                        }
                        process->status = FINISHED;
                        close(process->pipe_fd[0]);
                        process->end_time = time(NULL);                                                     // Registra o término da execução
                        process->execution_time = (int)(process->end_time - process->start_execution_time); // Calcula o tempo total de execução
                        sem_post(&scheduler->available_cores);                                              // Libera o núcleo
                        printf("Semáforo liberado pelo Processo %d\n", process->id);
                    }
                    else if (result == 0)
                    {
                        printf("Enviando SIGSTOP para Processo %d (PID %d)\n", process->id, process->pid);
                        process->status = SUSPENDED;
                    }
                    else
                    {
                        perror("Erro no waitpid");
                        process->status = FINISHED;            // Marca como finalizado em caso de erro
                        sem_post(&scheduler->available_cores); // Libera o núcleo
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
                printf("Processo %d: Turnaround = %d segundos, Tempo de Execução = %d segundos\n",
                       process->id, turnaround, process->execution_time);
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

    sem_destroy(&scheduler->available_cores); // Destroi o semáforo
}
