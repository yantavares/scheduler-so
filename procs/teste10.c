#include <stdio.h>
#include <time.h>

int main()
{
    // Obtém o tempo inicial
    time_t start_time = time(NULL);

    // Loop até atingir 10 segundos
    while (time(NULL) - start_time < 10)
    {
        // Um loop vazio que consome tempo
        for (volatile long i = 0; i < 1000000; i++)
        {
            // Adiciona volatilidade para evitar otimizações do compilador
        }
    }

    printf("Programa finalizado após 10 segundos.\n");
    return 0;
}
