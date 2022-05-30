/**
 * Copyright 2018 University of Applied Sciences Western Switzerland / Fribourg
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Project:	HEIA-FR / HES-SO MSE - MA-CSEL1 Laboratory
 *
 * Abstract: System programming -  file system
 *
 * Purpose:	exo1 travaux protatique ordonanceur
 *
 * Autĥor:	Gaetan Kolly, Andrea Enrile
 * Date:	25.mai.2022
 */
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sched.h>



#define MAX_BYTE_NUMBER 30
#define CMD_EXIT "exit\n"

/*
* handler signals
*/
static void catch_signal(int signal)
{
    printf("signal %d ignored\n", signal);
}
/*
* attach handler to signals
*/
static void install_catch_signal()
{
    struct sigaction act = {
        .sa_handler = catch_signal,
    };
    sigemptyset(&act.sa_mask);
    sigaction(SIGHUP, &act, NULL);   //  1 - hangup
    sigaction(SIGINT, &act, NULL);   //  2 - terminal interrupt
    sigaction(SIGQUIT, &act, NULL);  //  3 - terminal quit
    sigaction(SIGABRT, &act, NULL);  //  6 - abort
    sigaction(SIGTERM, &act, NULL);  // 15 - termination
    sigaction(SIGTSTP, &act, NULL);  // 19 - terminal stop signal
}

/*
* code run by the parent processs
*/
int fct_parent(int fd_socketpair){
  // conf CPU
  cpu_set_t set;
  CPU_SET(0, &set);
  if (sched_setaffinity(0, sizeof(cpu_set_t),&set)<0){
    perror("ERROR set affinity");
  }

  char readData[MAX_BYTE_NUMBER];
  int stop=0;
  // init epoll
  int epfd = epoll_create1(0);
  if (epfd == -1)
    perror("ERROR");
  struct epoll_event events={
    .events = EPOLLIN,
    .data.fd= fd_socketpair
  };
  int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd_socketpair, &(events));
  if (ret ==-1){
      perror("ERROR");
  } 

  // wait for exit, display in console
  while(!stop){
    epoll_wait(epfd, &events, 1, -1);
    int nb=read(fd_socketpair,readData,MAX_BYTE_NUMBER-1);
    if(nb<0){
      perror("read from parent");
    }
    else if(nb==0){
      printf("nothing to read\n");
    }
    else{
      readData[nb]=0;
      if(strcmp(readData,CMD_EXIT)==0){
        // exit
        stop=1;
      }
      else{
        printf("From parent: %s",readData);
      }
      
    }
  }
  return 0;
}
/*
* code run by the child processs
*/
int fct_child(int fd_socketpair){

  cpu_set_t set;
  CPU_SET(1, &set);
  if (sched_setaffinity(0, sizeof(cpu_set_t),&set)<0){
    perror("ERROR set affinity");
  }

  char writeData[MAX_BYTE_NUMBER];
  int stop =0;
  printf("child started, all lines entered will be transfer to the parent via socketpair\n");
  while(!stop){
    if (fgets(writeData, MAX_BYTE_NUMBER, stdin)) {
      if(write(fd_socketpair,writeData,strlen(writeData))<(ssize_t)strlen(writeData)){
        perror("Error writing into socket\n");
      }
      if(strcmp(writeData,CMD_EXIT)==0){
        stop=1;
      }
    }
  }
  return 0;
}


int main()
{
  // install handler to signal
  install_catch_signal();

  // redirect error pip
  int fd_errorlog=open("error.log", O_WRONLY | O_APPEND | O_CREAT, 0644);
  if(fd_errorlog<0){
    perror("Error opening log file");
  }
  // duplicate fd_errorlog to to stderr
  if (dup2(fd_errorlog, STDERR_FILENO) != STDERR_FILENO) { 
    perror("Error duplicating log file");
  }

  // test error
  perror("ERROR TEST");

  // Create socketpair
  int fd_socketpair[2];

  /* 
  * int socketpair (int domain, int type, int protocol, int fd[2]);
  * domain: AF_UNIX, local communication
  */
  int err = socketpair(AF_UNIX, SOCK_STREAM, 0, fd_socketpair);
  if (err <0){
    perror("Socketpair creation");
    return -1;
  }

  // clear CPU configuration
  cpu_set_t set;
  CPU_ZERO(&set);

  // forking
  pid_t pid = fork();
  if (pid == 0){
    // child
    close(fd_socketpair[0]);
    fct_child(fd_socketpair[1]);
    exit(0);
  } 
  else if (pid > 0){
    // parent
    close(fd_socketpair[1]);
    fct_parent(fd_socketpair[0]);
    // wait that the child finish
    int status = 0;
    waitpid(pid, &status, 0);
    //pid_t pid = waitpid (-1, &status, 0);
  }
  else{
    perror("Error fork");
  }

  return 0;
}