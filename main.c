#include "scheduler.h"

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(stderr, "Uso: %s <numero_de_cores> <quantum> <arquivo_entrada>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int num_cores = atoi(argv[1]);
    int quantum = atoi(argv[2]);
    const char *input_file = argv[3];

    if (num_cores <= 0)
    {
        fprintf(stderr, "Número de cores inválido.\n");
        return EXIT_FAILURE;
    }

    if (quantum <= 0)
    {
        fprintf(stderr, "Quantum inválido.\n");
        return EXIT_FAILURE;
    }

    Scheduler scheduler;
    init_scheduler(&scheduler, quantum, num_cores);

    execute_scheduler(&scheduler, input_file);

    return EXIT_SUCCESS;
}
