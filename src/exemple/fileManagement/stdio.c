#include <stdio.h>

int main(int argc, char* argv[])
{
    (void)argc;
    char buf[1];
    int c        = 0;
    FILE* stream = fopen(argv[1], "r");
    do {
        c = fread(buf, sizeof(buf), 1, stream);

    } while (c > 0);
    fclose(stream);

    return 0;
}
