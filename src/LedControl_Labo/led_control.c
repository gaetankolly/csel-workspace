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

  // open gpio value attribute
  sprintf(path,"%s%s/value",GPIO_GPIO,buttonNb);
  f = open(path, O_RDWR);
  return f;
}

int main(int argc, char *argv[])
{
  long duty = 2;      // %
  long period = 1000; // ms
  if (argc >= 2)
    period = atoi(argv[1]);
  period *= 1000000; // in ns

  // compute duty period...
  long p1 = period / 100 * duty;
  long p2 = period - p1;

  int led = open_led();
  pwrite(led, "1", sizeof("1"), 0);

  int k1= open_button(K1);
  int k2= open_button(K2);
  int k3= open_button(K3);

  struct timespec t1;
  clock_gettime(CLOCK_MONOTONIC, &t1);

  int k = 0;
  while (1)
  {
    struct timespec t2;
    clock_gettime(CLOCK_MONOTONIC, &t2);

    long delta =
        (t2.tv_sec - t1.tv_sec) * 1000000000 + (t2.tv_nsec - t1.tv_nsec);

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
      char test;
      pread(k1,&test,1,0);
      printf("button1= %c\n",test);
      pread(k2,&test,1,0);
      printf("button2= %c\n",test);
      pread(k3,&test,1,0);
      printf("button3= %c\n",test);
      
    }
  }

  return 0;
}