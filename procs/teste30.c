#include <stdio.h>
#include <time.h>

int main()
{
    // Obtém o tempo inicial
    time_t start_time = time(NULL);

    // Loop até atingir 30 segundos
    while (time(NULL) - start_time < 30)
    {
        // Um loop vazio que consome tempo
        for (volatile long i = 0; i < 3000000; i++)
        {
            // Adiciona volatilidade para evitar otimizações do compilador
        }
    }

    printf("Programa finalizado após 30 segundos.\n");
    return 0;
}
