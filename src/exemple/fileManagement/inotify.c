#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static void print_events(uint32_t mask)
{
    if (mask & IN_ACCESS)        printf(" access       ");
    if (mask & IN_ATTRIB)        printf(" attribute    ");
    if (mask & IN_CLOSE_WRITE)   printf(" close_write  ");
    if (mask & IN_CLOSE_NOWRITE) printf(" close_nowrite");
    if (mask & IN_CREATE)        printf(" create       ");
    if (mask & IN_DELETE)        printf(" delete       ");
    if (mask & IN_DELETE_SELF)   printf(" delete_self  ");
    if (mask & IN_MODIFY)        printf(" modify       ");
    if (mask & IN_MOVE_SELF)     printf(" move_self    ");
    if (mask & IN_MOVED_FROM)    printf(" move_from    ");
    if (mask & IN_MOVED_TO)      printf(" move_to      ");
    if (mask & IN_OPEN)          printf(" open         ");

    if (mask & IN_DONT_FOLLOW)   printf(" dont_follow  ");
    if (mask & IN_EXCL_UNLINK)   printf(" excl_unlink  ");
    if (mask & IN_IGNORED)       printf(" ignored      ");
    if (mask & IN_ISDIR)         printf(" is_dir       ");
    if (mask & IN_Q_OVERFLOW)    printf(" q_overflow   ");
    if (mask & IN_UNMOUNT)       printf(" unmount      ");
    printf("\n");
}

static void process_inotify(int fd)
{
    char buff[10 * (sizeof(struct inotify_event) + NAME_MAX + 1)];
    ssize_t sz = read(fd, buff, sizeof(buff));
    if (sz == -1) {
        perror("ERROR while reading inotify fd");
        if (errno == EINTR) return;
        exit(1);
    }

    printf("inotify event:\n");
    char* p = buff;
    while (sz > 0) {
        struct inotify_event* event = (struct inotify_event*)p;
        printf("  wd:%d, cookie=%d, mask=0x%x",
               event->wd,
               event->cookie,
               event->mask);
        print_events(event->mask);
        event->name[event->len] = 0;
        printf("  name:%s\n\n", event->name);
        p += sizeof(struct inotify_event) + event->len;
        sz -= sizeof(struct inotify_event) + event->len;
    }
}

int main(int argc, char* argv[])
{
    char* path = ".";
    if (argc > 1) path = argv[1];

    int efd = epoll_create1(0);
    if (efd == -1) {
        perror("ERROR while create epoll");
        exit(1);
    }

    int ifd = inotify_init1(0);
    if (ifd == -1) {
        perror("ERROR while creating inotify file descriptor");
        exit(1);
    }
    struct epoll_event event = {
        .events  = EPOLLIN,
        .data.fd = ifd,
    };
    epoll_ctl(efd, EPOLL_CTL_ADD, ifd, &event);

    int wd1 = inotify_add_watch(ifd, path, IN_ALL_EVENTS);
    if (wd1 == -1) {
        perror("ERROR while adding directory to inotify fd");
        exit(1);
    }

    while (1) {
        struct epoll_event events[2];
        int ret = epoll_wait(efd, events, 2, -1);
        if (ret == -1) {
            perror(" --> error...");

        } else if (ret == 0) {
            perror(" --> nothing received...");

        } else {
            for (int i = 0; i < ret; i++) {
                int fd = events[i].data.fd;
                if (fd == ifd)
                    process_inotify(fd);
                else
                    printf("wrong fd..\n");
            }
        }
    }

    return 0;
}
