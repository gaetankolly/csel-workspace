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

/**
 * convert filetype to character
 */
char mode2filetype(mode_t mode)
{
    // process file type
    if ((mode & S_IFMT) == S_IFSOCK) return 's';
    if ((mode & S_IFMT) == S_IFLNK) return 'l';
    if ((mode & S_IFMT) == S_IFREG) return '-';
    if ((mode & S_IFMT) == S_IFBLK) return 'b';
    if ((mode & S_IFMT) == S_IFDIR) return 'd';
    if ((mode & S_IFMT) == S_IFCHR) return 'c';
    if ((mode & S_IFMT) == S_IFIFO) return 'f';
    return '-';
}

/**
 * convert file permissions to string
 */
char* mode2str(mode_t mode)
{
    static char buf[1 + 3 * 3 + 1 + 1];

    buf[0] = mode2filetype(mode);

    buf[1] = mode & S_IRUSR ? 'r' : '-';
    buf[2] = mode & S_IWUSR ? 'w' : '-';
    buf[3] = mode & S_IXUSR ? 'x' : '-';

    buf[4] = mode & S_IRGRP ? 'r' : '-';
    buf[5] = mode & S_IWGRP ? 'w' : '-';
    buf[6] = mode & S_IXGRP ? 'x' : '-';

    buf[7] = mode & S_IROTH ? 'r' : '-';
    buf[8] = mode & S_IWOTH ? 'w' : '-';
    buf[9] = mode & S_IXOTH ? 'x' : '-';

    buf[10] = mode & S_ISVTX ? 't' : '-';
    if (mode & S_ISGID) buf[6] = 's';
    if (mode & S_ISUID) buf[3] = 's';

    buf[11] = 0;

    return buf;
}

/**
 * convert file time to string
 */
char* time2str(time_t* time)
{
    static char buf[50];
    char* t          = ctime_r(time, buf);
    t[strlen(t) - 1] = 0;
    return t;
}

/**
 * print file information
 *
 *@param name file name
 *@param stat file status
 */
void print_info(const char* name, struct stat* stat)
{
    printf("%10lu", stat->st_ino);             // print inode number
    printf(" %s", mode2str(stat->st_mode));    // print file type & permissions
    printf(" %3d", stat->st_nlink);            // print number of links
    printf(" %8lu", stat->st_size);            // print file size
    printf(" %s", time2str(&stat->st_mtime));  // print file modification time
    printf(" %s\n", name);                     // print filename
}

/**
 * scan directory for files and subdirectories
 *
 *@param recursively true to scan recursively the directories
 *@param size of root path
 *@param path directory path to scan
 */
void scan_dir(bool recursively, int rootsz, const char* path)
{
    // print relative directory path
    printf("\n.%s/:\n", path);

    // count number of files entries in directory
    DIR* dir = opendir(".");
    if (dir == 0) return;
    int count = 0;
    while (dir != 0) {
        struct dirent* entry = readdir(dir);
        if (entry == 0) break;
        count++;
    }
    closedir(dir);

    // get all file names
    char files[count * 256];
    dir = opendir(".");
    if (dir == 0) return;
    int i = 0;
    while (dir != 0) {
        struct dirent* entry = readdir(dir);
        if (entry == 0) break;

        strcpy(files + i * 256, entry->d_name);
        i++;
    }
    closedir(dir);

    // sort alphabeticaly all files
    qsort(files, count, 256, (int (*)(const void*, const void*))strcmp);

    // print file information
    for (int j = 0; j < count; j++) {
        struct stat status;
        int err = stat(files + j * 256, &status);
        if (err < 0) continue;
        print_info(files + j * 256, &status);
    }

    // process subdirectories if required...
    if (recursively)
        for (int j = 0; j < count; j++) {
            char* file = files + j * 256;
            if (strcmp(file, ".") == 0) continue;
            if (strcmp(file, "..") == 0) continue;

            struct stat status;
            int err = stat(file, &status);
            if (err < 0) continue;
            if (!S_ISDIR(status.st_mode)) continue;

            char p[strlen(path) + 1 + 256];
            strcpy(p, path);
            strcat(p, "/");
            strcat(p, file);
            chdir(file);
            scan_dir(recursively, rootsz, p);
            chdir("..");
        }
}

/**
 * print application usage
 */
static void print_usage()
{
    printf(
        "Usage: list [OPTIONs] [PATH]\n"
        "List information about files in directories\n"
        "Options:\n"
        "  -h   this message\n"
        "  -r   recursive to list subdirectories\n"
        "Path:\n"
        "  by default current directory\n");
}

/**
 * main program...
 */
int main(int argc, char* argv[])
{
    printf("List files in directories...\n");

    // parse fonction arguments
    bool recursively = false;
    const char* path = ".";
    while (1) {
        int op = getopt(argc, argv, "hr");
        if (op == -1) break;
        switch (op) {
            case 'h':
                print_usage();
                return 0;
            case 'r':
                recursively = true;
                break;
            default:
                fprintf(stderr, "ERROR: invalid option.\n");
                exit(1);
        }
    }
    int r = argc - optind;
    if (r == 1)
        path = argv[optind];
    else if (r > 1) {
        fprintf(stderr, "ERROR: too many arguments\n");
        exit(1);
    }
    printf("options: r=%d, p=%s\n", recursively, path);

    char* cwd = getcwd(0, 0);
    chdir(path);
    scan_dir(recursively, strlen(cwd), path);
    free(cwd);

    return 0;
}
