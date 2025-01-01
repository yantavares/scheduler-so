#include <stdio.h>
#include <unistd.h>

int main()
{
    for (long i = 0; i < 1800000000; i++)
        ; // Simula 30 segundos
    printf("Processo teste30 concluído.\n");
    return 0;
}
