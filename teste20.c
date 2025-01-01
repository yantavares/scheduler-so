#include <stdio.h>
#include <unistd.h>

int main()
{
    for (long i = 0; i < 1200000000; i++)
        ; // Simula 20 segundos
    printf("Processo teste20 concluído.\n");
    return 0;
}
