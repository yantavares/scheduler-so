#include <stdio.h>
#include <time.h>

int main()
{
    long long i;         // Variável para o loop
    clock_t inicio, fim; // Variáveis para medir o tempo

    inicio = clock();

    // Loop com número grande de iterações
    for (i = 0; i < 20000000000LL; i++)
        ;

    // Marcar o fim do timer
    fim = clock();

    // Calcular o tempo total em segundos
    double tempo_total = (double)(fim - inicio) / CLOCKS_PER_SEC;

    // Exibir resultados
    printf("Loop finalizado. Número total de iterações: %lld\n", i);
    printf("Tempo total do loop: %.2f segundos\n", tempo_total);

    return 0;
}
