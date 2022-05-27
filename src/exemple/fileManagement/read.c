#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// dd if=/dev/urandom of=./t.txt bs=4k count=4096

int main(int argc, char* argv[])
{
    const char* fn = "test1.txt";
    if (argc > 1) fn = argv[1];

    int fd = open(fn, O_RDONLY);
    if (fd == -1) {
        printf("file not found!\n");
    }

    while (1) {
        char buf[4096 + 1];
        int ln = read(fd, buf, sizeof(buf) - 1);
        if (ln == -1) {
            if (errno == EINTR) continue;
            break;
        }

        if (ln == 0) break;

        buf[ln] = 0;
        printf("%s\nread length --> %d\n", buf, ln);
    };
    close(fd);

    return 0;
}
