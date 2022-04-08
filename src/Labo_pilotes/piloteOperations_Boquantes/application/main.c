#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <unistd.h>

int main(void)
{
  int fd;
  int counter = 0;
  fd_set fd_in;

  // open virtual file
  fd = open("/dev/mymodule", O_RDWR);
  if (fd == -1){
    printf("Error while opening /dev/mymodule\n");
    return 0;
  }
  else{
    printf("Device /dev/mymodule opened successfully!\n");
  }

  FD_ZERO(&fd_in);

  while (1){

    FD_SET(fd, &fd_in);
    int n = select(fd + 1, &fd_in, NULL, NULL, NULL);  // fd + 1 is needed regarding "man" pages
    if (n == -1){
      perror("Error using select.");
    }
    else if (FD_ISSET(fd, &fd_in)){
      counter++;
      printf("Counter=%d\n", counter);
    }
  }
  close(fd);
}