#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
static const struct {
    off_t pos;
    off_t offset;
    int whence;
} tests[] = {
    {0, 0, SEEK_SET},
    {10, 10, SEEK_SET},
    {21, 10, SEEK_CUR},
    {12, -10, SEEK_CUR},
    {1000, -1, SEEK_END},
};

static const char* i2w[] = {
    [SEEK_SET] = "SEEK_SET",
    [SEEK_CUR] = "SEEK_CUR",
    [SEEK_END] = "SEEK_END",
};

int main(int argc, char* argv[])
{
    const char* fn = "test2.txt";
    if (argc > 1) fn = argv[1];

    int fd = open(fn, O_RDWR | O_CREAT | O_TRUNC, 0664);
    if (fd == -1) {
        printf("can't file!\n");
    }

    // 1st create a file with 1001 entries 0000 - 1000
    char entry[5 + 3];
    for (int i = 0; i <= 1000; i++) {
        snprintf(entry, sizeof(entry) - 1, "%04d\n", i);
        write(fd, entry, strlen(entry));
    }

    for (unsigned i = 0; i < ARRAY_SIZE(tests); i++) {
        off_t pos = lseek(fd, tests[i].offset * 5, tests[i].whence);
        read(fd, entry, 5);
        entry[4] = 0;
        printf("%s: entry=%s, offset: %3ld, pos: %ld/%ld: \n",
               i2w[tests[i].whence],
               entry,
               tests[i].offset,
               tests[i].pos,
               pos / 5);
    }

    close(fd);
}
