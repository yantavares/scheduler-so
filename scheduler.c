#include "scheduler.h"

/**---------------------------------------------------------
 * Autores:
 * * Yan Tavares - 202014323
 * * Guilherme Soares
 * * Gustavo Valentim
 * * Gabriel Farago
 *
 * Versão do Compilador: GCC 14.2.1
 * Sistema Operacional: Arch Linux x86_64
 * Kernel: 6.12.8-arch1-1
 * ------------------------------------------------------ */

/**
 * @brief Imprime os IDs dos processos na fila de prioridade.
 *
 * @param queue Ponteiro para a fila de prioridade.
 */
void print_priority_queue(PriorityQueue *queue)
{
    for (int i = 0; i < queue->count; i++)
    {
        Process *process = &queue->processes[i];
        printf("p%d ", process->id);
    }
    printf("\n");
}

/**
 * @brief Verifica se algum processo já chegou na fila e atualiza seu status.
 *
 * @param scheduler Ponteiro para o escalonador.
 * @param current_time Tempo atual.
 * @return Retorna a prioridade do primeiro processo que chegou ou -1 se nenhum chegou.
 */
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

/**
 * @brief Inicializa o escalonador com os valores iniciais.
 *
 * @param scheduler Ponteiro para o escalonador.
 * @param quantum Quantum de tempo para cada processo.
 * @param num_cores Número de núcleos disponíveis.
 */
void init_scheduler(Scheduler *scheduler, int quantum, int num_cores)
{
    scheduler->quantum = quantum;
    for (int i = 0; i < MAX_PRIORITY; i++)
    {
        scheduler->queues[i].count = 0;
    }
    sem_init(&scheduler->available_cores, 0, num_cores);
}

/**
 * @brief Adiciona um processo à fila de prioridade correspondente no escalonador.
 *
 * @param scheduler Ponteiro para o escalonador.
 * @param process Ponteiro para o processo que será adicionado.
 */
void add_process(Scheduler *scheduler, Process *process)
{
    PriorityQueue *queue = &scheduler->queues[process->priority];
    queue->processes[queue->count++] = *process;
}

/**
 * @brief Executa um processo criando um novo processo filho e redireciona a saída para o pipe.
 *
 * @param process Ponteiro para o processo a ser executado.
 * @param quantum Quantum de tempo alocado para o processo.
 * @param pipe_fd Descritor do pipe para comunicação entre processos.
 */
void execute_process(Process *process, int quantum, int pipe_fd)
{
    pid_t pid = fork();

    if (pid == 0)
    {
        close(pipe_fd);
        dup2(pipe_fd, STDOUT_FILENO);

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

/**
 * @brief Lê mensagens do pipe e imprime no console. Atualiza status de processos conforme necessário.
 *
 * @param pipe_fd Descritor do pipe.
 */
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

/**
 * @brief Executa o escalonador lendo processos do arquivo de entrada e gerenciando a execução.
 *
 * @param scheduler Ponteiro para o escalonador.
 * @param input_file Nome do arquivo de entrada contendo os processos.
 */
void execute_scheduler(Scheduler *scheduler, const char *input_file)
{
    signal(SIGSTOP, SIG_IGN);
    signal(SIGCONT, SIG_IGN);

    time_t start_time_global = time(NULL);

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

    fcntl(pipe_fd[0], F_SETFL, O_NONBLOCK);

    int running = 1;

    while (running)
    {
        running = 0;
        time_t current_time = time(NULL);

        for (int priority = 0; priority < MAX_PRIORITY; priority++)
        {
            PriorityQueue *queue = &scheduler->queues[priority];
            int original_count = queue->count;

            int restart_flag = 0;
            for (int i = 0; i < original_count; i++)
            {
                print_priority_queue(queue);
                int arrived = has_any_process_arrived(scheduler, time(NULL) - start_time_global);
                if (arrived != -1)
                {
                    priority = -1;
                    restart_flag = 1;
                }

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

                        read_pipe_and_update_status(pipe_fd[0]);

                        int arrived = has_any_process_arrived(scheduler, time(NULL) - start_time_global);
                        if (arrived != -1)
                        {
                            priority = -1;
                            restart_flag = 1;
                        }
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
                if (restart_flag)
                {
                    break;
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
