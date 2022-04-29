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
 * Autĥor:	Daniel Gachet
 * Date:	07.11.2018
 */
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

/*
 * status led - gpioa.10 --> gpio10
 * power led  - gpiol.10 --> gpio362
 */
#define GPIO_EXPORT "/sys/class/gpio/export"
#define GPIO_UNEXPORT "/sys/class/gpio/unexport"
#define GPIO_LED "/sys/class/gpio/gpio10"
#define LED "10"

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
    }
  }

  return 0;
}