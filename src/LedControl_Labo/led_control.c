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
 * Purpose:	NanoPi silly status led control system
 *
 * Autĥor:	Gaetan Kolly, Andrea Enrile
 * Date:	29.04.2022
 */
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>

/*
 * status led - gpioa.10 --> gpio10
 * power led  - gpiol.10 --> gpio362
 */
#define GPIO_EXPORT "/sys/class/gpio/export"
#define GPIO_UNEXPORT "/sys/class/gpio/unexport"
#define GPIO_LED "/sys/class/gpio/gpio10"
#define GPIO_GPIO "/sys/class/gpio/gpio"
#define LED "10"
#define K1 "0"
#define K2 "2"
#define K3 "3"
#define NB_EPOLL_EVENT 4
#define BUTTON_STEP 50000000 //ns
//#define INTERRUPT_BOTH ("both")
#define INTERRUPT_RISING ("rising")

struct button{
  int fd_k;
  int state;
};

/*
 * init led 10, return fd /value
 */
static int open_led()
{
  // unexport pin out of sysfs (reinitialization)
  int f = open(GPIO_UNEXPORT, O_WRONLY);
  write(f, LED, strlen(LED));
  close(f);

  // export pin to sysfs
  f = open(GPIO_EXPORT, O_WRONLY);
  write(f, LED, strlen(LED));
  close(f);

  // config pin
  f = open(GPIO_LED "/direction", O_WRONLY);
  write(f, "out", 3);
  close(f);

  // open gpio value attribute
  f = open(GPIO_LED "/value", O_RDWR);
  return f;
}

/*
 * init buttonNb, return fd /value
 */
static int open_button(char* buttonNb)
{
  char path[50]={0};
  int strLen= strlen(buttonNb);
  if (strLen>3){
    return -1;
  }

  // unexport pin out of sysfs (reinitialization)
  int f = open(GPIO_UNEXPORT, O_WRONLY);
  write(f, buttonNb, strLen);
  close(f);

  // export pin to sysfs
  f = open(GPIO_EXPORT, O_WRONLY);
  write(f, buttonNb, strlen(buttonNb));
  close(f);

  // config pin
  sprintf(path,"%s%s/direction",GPIO_GPIO,buttonNb);
  f = open(path, O_WRONLY);
  write(f, "in", 2);
  close(f);

  // config event, rising edge
#ifndef INTERRUPT_BOTH
  sprintf(path,"%s%s/edge",GPIO_GPIO,buttonNb);
  f = open(path, O_WRONLY);
  write(f, "INTERRUPT_RISING", strlen("INTERRUPT_RISING"));
  close(f);
#else
  sprintf(path, "%s%s/edge", GPIO_GPIO, buttonNb);
  f = open(path, O_WRONLY);
  write(f, "INTERRUPT_BOTH", strlen("INTERRUPT_BOTH"));
  close(f);
#endif
  
  // open gpio value attribute
  sprintf(path,"%s%s/value",GPIO_GPIO,buttonNb);
  f = open(path, O_RDWR);
  

  return f;
}

void setTime_ns(long time_ns,struct timespec* p_time_val){
  p_time_val->tv_sec=(long) (time_ns/1000000000);
  p_time_val->tv_nsec=time_ns%1000000000;
}

void setDutyCycle(long period,int duty,long* p1, long* p2){
  // compute duty period...
  *p1 = period / 100 * duty;
  *p2 = period - *p1;
}

int main(int argc, char *argv[])
{
  
  long duty = 50;      // %
  long period = 2000; // ms
  if (argc >= 2)
    period = atoi(argv[1]);
  period *= 1000000; // in ns
  long periodModif=period;
  long p1,p2;
  setDutyCycle(periodModif,duty,&p1, &p2);

  //long ms_interval= 500; //500ms

  int led = open_led();
  pwrite(led, "1", sizeof("1"), 0);

  // init button
  struct button myButtons[3];
  myButtons[0].fd_k = open_button(K1);
  myButtons[1].fd_k = open_button(K2);
  myButtons[2].fd_k = open_button(K3);
  // int fd_k[3];
  // fd_k[0]= open_button(K1);
  // fd_k[1]= open_button(K2);
  // fd_k[2]= open_button(K3);
#ifndef INTERRUPT_BOTH
#else
  int activeButton = 0;
#endif
  

  // init epoll
  int epfd = epoll_create1(0);
  if (epfd == -1)
    perror("ERROR");
  
  struct epoll_event events_ctl_button[3];
  for(int i=0; i<3;i++){
    events_ctl_button[i].events=EPOLLET;
    events_ctl_button[i].data.fd = myButtons[i].fd_k; // to read after
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, myButtons[i].fd_k, &(events_ctl_button[i]));
    if (ret ==-1)
      perror("ERROR");
  }
  
  // Init timer
  //struct timespec t1;
  //clock_gettime(CLOCK_MONOTONIC, &t1);
  int timerfd = timerfd_create(CLOCK_MONOTONIC, 0); 
  struct itimerspec new_value;
  setTime_ns(p1,&new_value.it_value);
  //setTime_ns(1000000000,&new_value.it_interval); // doesnt work... why?
  int ret=timerfd_settime(timerfd, 0, &new_value, NULL);
  if (ret ==-1)
      perror("ERROR");

  // add to epoll event
  struct epoll_event events_ctl_timer;
  events_ctl_timer.events=EPOLLIN | EPOLLET;
  events_ctl_timer.data.fd=timerfd; //to read after
  ret = epoll_ctl(epfd,EPOLL_CTL_ADD,timerfd,&events_ctl_timer);
  if (ret ==-1)
      perror("ERROR");
  int toggle=0;

  //int k = 0;
  while (1)
  {
    /*struct timespec t2;
    clock_gettime(CLOCK_MONOTONIC, &t2);

    long delta =
        (t2.tv_sec - t1.tv_sec) * 1000000000 + (t2.tv_nsec - t1.tv_nsec);
    */
    struct epoll_event events[NB_EPOLL_EVENT];
    int nr = epoll_wait(epfd, events, NB_EPOLL_EVENT, -1);
    if (nr == -1)
      perror("ERROR");
    for (int i=0; i<nr; i++) {
      //printf ("event=%d on fd=%d\n", events[i].events, events[i].data.fd);
      // manage buttons
#ifndef INTERRUPT_BOTH
      for (int j=0; j<3;j++){
        if (events[i].data.fd == myButtons[j].fd_k)
        {
          //printf("Button: %d\n",j+1);
          if(j==0){
            periodModif+=BUTTON_STEP;
          }
          else if(j==1){
            periodModif=period;
          }
          else if(j==2){
            periodModif-=BUTTON_STEP;
            if (periodModif<BUTTON_STEP){
              periodModif=BUTTON_STEP;
            }
          }
          setDutyCycle(periodModif,duty,&p1, &p2);
          printf("Period= %fms\n",(double)periodModif/1000000);
        }
      }
#else
      activeButton = myButtons[0].state | myButtons[1].state | myButtons[2].state;
      for (int j=0; j<3;j++){
        if (events[i].data.fd == myButtons[j].fd_k && activeButton != 1)
        {
          myButtons[j].state = 1;
          break;
        }
      }
      if (activeButton){
        for (int j = 0; j < 3; j++)
        {
          if (myButtons[j].state == 1)
          {
            // printf("Button: %d\n",j+1);
            if (j == 0)
            {
              periodModif += BUTTON_STEP;
            }
            else if (j == 1)
            {
              periodModif = period;
            }
            else if (j == 2)
            {
              periodModif -= BUTTON_STEP;
              if (periodModif < BUTTON_STEP)
              {
                periodModif = BUTTON_STEP;
              }
            }
            setDutyCycle(periodModif, duty, &p1, &p2);
            printf("Period= %fms\n", (double)periodModif / 1000000);
            msleep(250);
          }
        }
      }
#endif

      // manage timer
      if (events[i].data.fd==timerfd){
        if (toggle){
          pwrite(led, "1", sizeof("1"), 0);
          setTime_ns(p1,&new_value.it_value);
          timerfd_settime(timerfd, 0, &new_value, NULL);
        }
          
        else{
          pwrite(led, "0", sizeof("0"), 0);
          setTime_ns(p2,&new_value.it_value);
          timerfd_settime(timerfd, 0, &new_value, NULL);
        }
          
        toggle=!toggle;
      }
    }

    /*
    int toggle = ((k == 0) && (delta >= p1)) | ((k == 1) && (delta >= p2));
    if (toggle)
    {
      t1 = t2;
      k = (k + 1) % 2;
      if (k == 0)
        pwrite(led, "1", sizeof("1"), 0);
      else
        pwrite(led, "0", sizeof("0"), 0);

      // test button
      // ssize_t pread (int fd, void* buf, size_t count, off_t pos);
      // char test;
      // pread(k1,&test,1,0);
      // printf("button1= %c\n",test);
      // pread(k2,&test,1,0);
      // printf("button2= %c\n",test);
      // pread(k3,&test,1,0);
      // printf("button3= %c\n",test);
      // wait events
    }
    */

  }

  return 0;
}