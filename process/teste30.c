#include <stdio.h>

int main()
{
    long long i;

    for (i = 0; i < 125000000000LL; i++)
        ;

    printf("Loop finalizado. Número total de iterações: %lld\n", i);

    return 0;
}
