#include <stdio.h>
#include <unistd.h>

int main()
{
    for (int i = 0; i < 5; i++)
    {
        printf("Teste10 progresso: %d/5\n", i + 1);
        fflush(stdout);
        sleep(2); // Simula trabalho
    }
    return 0;
}
