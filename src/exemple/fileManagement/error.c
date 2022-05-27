#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    printf("Test perror vs strerror...\n");

    char* filename = "./error.txt";
    if (argc >= 2) filename = argv[1];

    int fd = open(filename, O_RDWR);

    printf("fd = %d, errno=%d\n", fd, errno);

    printf("\ntest perror...\n");
    perror("Error");

    printf("\ntest strerror_r...\n");
    char estr[100] = "";
    strerror_r(errno, estr, sizeof(estr) - 1);
    fprintf(stderr, "Error: %s\n", estr);

    return 0;
}
