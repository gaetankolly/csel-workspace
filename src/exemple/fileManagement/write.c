#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static const char* msg1 = "1234567890abcdefghijklmnopqrstuvwxyz\n";

int main(int argc, char* argv[])
{
    const char* fn = "test1.txt";
    if (argc > 1) fn = argv[1];

    int fd = open(fn, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        printf("can't file!\n");
    }

    for (int i = 0; i < 100; i++) {
        write(fd, msg1, strlen(msg1));
    }
    close(fd);
}
