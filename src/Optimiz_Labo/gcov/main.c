#include <stdio.h>

int main (void)
{
    int res = 1;
    for (int i=0; i<16; i++)
    {
        res *= 2;
    }

    if(res < 10)
    {
        printf("dead code\n");
    }

    printf("res=%d\n", res);

    return 0;
}
