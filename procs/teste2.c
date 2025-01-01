#include <stdio.h>
#include <time.h>

int main()
{
    // Obtém o tempo inicial
    time_t start_time = time(NULL);

    // Loop até atingir 2 segundos
    while (time(NULL) - start_time < 2)
    {
        // Um loop vazio que consome tempo
        for (volatile long i = 0; i < 200000; i++)
        {
            // Adiciona volatilidade para evitar otimizações do compilador
        }
    }

    printf("Programa finalizado após 2 segundos.\n");
    return 0;
}
