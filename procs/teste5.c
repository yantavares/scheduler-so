#include <stdio.h>
#include <time.h>

int main()
{
    // Obtém o tempo inicial
    time_t start_time = time(NULL);

    // Loop até atingir 5 segundos
    while (time(NULL) - start_time < 5)
    {
        // Um loop vazio que consome tempo
        for (volatile long i = 0; i < 500000; i++)
        {
            // Adiciona volatilidade para evitar otimizações do compilador
        }
    }

    printf("Programa finalizado após 5 segundos.\n");
    return 0;
}
