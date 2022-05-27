#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

void scan_dir(const char* path)
{
    printf("\n.%s/:\n", path);

    DIR* dir = opendir(path);
    if (dir == 0) return;
    while (dir != 0) {
        struct dirent* entry = readdir(dir);
        if (entry == 0) break;
        printf("%4d - %s\n", entry->d_type, entry->d_name);
    }
    closedir(dir);
}
int main(int argc, char* argv[])
{
    char* path = ".";
    if (argc > 1) path = argv[1];

    scan_dir(path);

    return 0;
}

