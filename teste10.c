#include <stdio.h>
#include <unistd.h>

int main()
{
    for (long i = 0; i < 600000000; i++)
        ; // Simula 10 segundos
    printf("Processo teste10 concluído.\n");
    return 0;
}
