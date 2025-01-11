#include "scheduler.h"

void print_priority_queue(PriorityQueue *queue)
{
    for (int i = 0; i < queue->count; i++)
    {
        Process *process = &queue->processes[i];
        printf("p%d ", process->id);
    }
    printf("\n");
}

int has_any_process_arrived(Scheduler *scheduler, time_t current_time)
{
    for (int priority = 0; priority < MAX_PRIORITY; priority++)
    {
        PriorityQueue *queue = &scheduler->queues[priority];
        for (int i = 0; i < queue->count; i++)
        {
            Process *process = &queue->processes[i];
            if (process->start_time > (int)(current_time))
            {
                continue;
            }
            else if (process->status == NOT_HERE)
            {
                printf("Processo %d chegou.\n", process->id);
                process->status = READY;
                return process->priority;
            }
        }
    }
    return -1;
}

// Inicializa o escalonador
void init_scheduler(Scheduler *scheduler, int quantum, int num_cores)
{
    scheduler->quantum = quantum;
    for (int i = 0; i < MAX_PRIORITY; i++)
    {
        scheduler->queues[i].count = 0;
    }
    sem_init(&scheduler->available_cores, 0, num_cores);
}

// Adiciona um processo à fila de prioridade correspondente
void add_process(Scheduler *scheduler, Process *process)
{
    PriorityQueue *queue = &scheduler->queues[process->priority];
    queue->processes[queue->count++] = *process;
}

// Executa um processo e configura o pipe para comunicação
void execute_process(Process *process, int quantum, int pipe_fd)
{
    pid_t pid = fork();

    if (pid == 0) // Processo filho
    {
        close(pipe_fd);               // Fecha a escrita no pipe no pai
        dup2(pipe_fd, STDOUT_FILENO); // Redireciona stdout para o pipe

        execl(process->executable, process->executable, NULL);
        perror("Erro ao executar o arquivo");
        exit(EXIT_FAILURE);
    }
    else if (pid > 0)
    {
        process->pid = pid;
    }
    else
    {
        perror("Erro no fork");
        exit(EXIT_FAILURE);
    }

    process->last_execution_start = time(NULL);
    process->status = RUNNING;
}

void read_pipe_and_update_status(int pipe_fd)
{
    char buffer[256];
    ssize_t bytes_read;

    while ((bytes_read = read(pipe_fd, buffer, sizeof(buffer) - 1)) > 0)
    {
        buffer[bytes_read] = '\0';
        printf("Mensagem do Pipe: %s", buffer);
    }

    if (bytes_read == -1 && errno != EAGAIN)
    {
        perror("Erro na leitura do pipe");
    }
}

void execute_scheduler(Scheduler *scheduler, const char *input_file)
{
    signal(SIGSTOP, SIG_IGN);
    signal(SIGCONT, SIG_IGN);

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
        process.status = NOT_HERE;
        process.start_time_original = process.start_time;
        process.execution_time = 0;
        process.last_execution_start = 0;
        printf("Processo %d - Executável: %s, Início: %d, Prioridade: %d\n",
               process.id, process.executable, process.start_time, process.priority);
        add_process(scheduler, &process);
    }
    fclose(file);

    printf("\nIniciando o escalonador...\n");

    int pipe_fd[2];
    if (pipe(pipe_fd) == -1)
    {
        perror("Erro ao criar o pipe");
        exit(EXIT_FAILURE);
    }

    fcntl(pipe_fd[0], F_SETFL, O_NONBLOCK); // Configura o pipe como não bloqueante

    int running = 1;

    while (running)
    {
        running = 0;
        time_t current_time = time(NULL);

        for (int priority = 0; priority < MAX_PRIORITY; priority++)
        {
            PriorityQueue *queue = &scheduler->queues[priority];
            int original_count = queue->count;

            for (int i = 0; i < original_count; i++)
            {
                print_priority_queue(queue);

                has_any_process_arrived(scheduler, time(NULL) - start_time_global);

                Process *process = &queue->processes[i];

                if (process->status == FINISHED)
                {
                    continue;
                }

                running = 1;

                if (process->status == READY)
                {
                    if (sem_trywait(&scheduler->available_cores) == 0)
                    {
                        printf("Semáforo obtido para Processo %d\n", process->id);
                        execute_process(process, scheduler->quantum, pipe_fd[1]);
                        printf("Processo %d iniciado: PID %d\n", process->id, process->pid);
                    }
                    else
                    {
                        printf("Semáforo não disponível para Processo %d\n", process->id);
                        continue;
                    }
                }

                if (process->status == RUNNING || process->status == SUSPENDED)
                {
                    if (process->status == SUSPENDED)
                    {
                        if (sem_trywait(&scheduler->available_cores) != 0)
                        {
                            printf("Semáforo não disponível para Processo %d\n", process->id);
                            continue;
                        }
                        process->last_execution_start = time(NULL);
                    }

                    printf("Enviando SIGCONT para Processo %d (PID %d)\n", process->id, process->pid);
                    kill(process->pid, SIGCONT);

                    time_t start_time = time(NULL);
                    int process_finished = 0;

                    while (time(NULL) - start_time < scheduler->quantum && !process_finished)
                    {
                        int status;
                        pid_t result = waitpid(process->pid, &status, WNOHANG);

                        if (result > 0)
                        {
                            if (WIFEXITED(status))
                            {
                                process->execution_time += (int)(time(NULL) - process->last_execution_start);
                                printf("Processo %d finalizou normalmente.\n", process->id);
                                process->status = FINISHED;
                                process->end_time = time(NULL);
                                sem_post(&scheduler->available_cores);
                                printf("Semáforo liberado pelo Processo %d\n", process->id);
                                process_finished = 1;
                            }
                        }

                        read_pipe_and_update_status(pipe_fd[0]); // Lê do pipe enquanto monitora

                        has_any_process_arrived(scheduler, time(NULL) - start_time_global);
                    }

                    if (!process_finished)
                    {
                        printf("Enviando SIGSTOP para Processo %d (PID %d)\n", process->id, process->pid);
                        process->execution_time += (int)(time(NULL) - process->last_execution_start);
                        kill(process->pid, SIGSTOP);
                        process->status = SUSPENDED;

                        Process temp = *process;
                        for (int j = i; j < queue->count - 1; j++)
                        {
                            queue->processes[j] = queue->processes[j + 1];
                        }
                        queue->processes[queue->count - 1] = temp;
                        i = -1;

                        sem_post(&scheduler->available_cores);
                    }
                }
            }
        }
    }

    printf("\nTodos os processos foram concluídos.\n");

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

    close(pipe_fd[0]);
    close(pipe_fd[1]);
    sem_destroy(&scheduler->available_cores);
}
