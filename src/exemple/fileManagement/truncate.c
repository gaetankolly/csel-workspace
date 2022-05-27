#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// echo "jslfjsdljslfjsdlfj" > t.txt

int main(int argc, char* argv[])
{
    char* fname = "t.txt";
    off_t len   = 10;
    if (argc > 1) fname = argv[1];

    int fd = open(fname, O_RDWR, 0664);
    if (fd == -1) {
        printf("can't open file: %s\n", fname);
    }
    printf("argv1: %s, fd=%d, errno=%d\n", fname, fd, errno);
    ftruncate(fd, len);
    close(fd);

    return 0;
}
