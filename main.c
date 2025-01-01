#include "scheduler.h"

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Uso: %s <quantum> <arquivo_entrada>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int quantum = atoi(argv[1]);
    const char *input_file = argv[2];

    if (quantum <= 0)
    {
        fprintf(stderr, "Quantum inválido.\n");
        return EXIT_FAILURE;
    }

    Scheduler scheduler;
    init_scheduler(&scheduler, quantum);

    execute_scheduler(&scheduler, input_file);

    return EXIT_SUCCESS;
}
