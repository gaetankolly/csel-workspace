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
 * Purpose:	funciton realted to gpio
 *
 * Autĥor:	Gaetan Kolly, Andrea Enrile
 * Date:	09.06.2022
 */
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "daemonCore.h"

#include "gpio.h"

#define GPIO_EXPORT "/sys/class/gpio/export"
#define GPIO_UNEXPORT "/sys/class/gpio/unexport"
#define GPIO_GPIO "/sys/class/gpio/gpio"
#define INTERRUPT_RISING "rising"
#define LED_POWER "10"

int fd_led;

/*
 * init led 10, return fd /value
 */
int init_led()
{
    char path[50]={0};
    // unexport pin out of sysfs (reinitialization)
    int f = open(GPIO_UNEXPORT, O_WRONLY);
    write(f, LED_POWER, strlen(LED_POWER));
    close(f);

    // export pin to sysfs
    f = open(GPIO_EXPORT, O_WRONLY);
    write(f, LED_POWER, strlen(LED_POWER));
    close(f);

    // config pin
    sprintf(path,"%s%s/direction",GPIO_GPIO,LED_POWER);
    f = open(path, O_WRONLY);
    write(f, "out", 3);
    close(f);

    // open gpio value attribute
    sprintf(path,"%s%s/value",GPIO_GPIO,LED_POWER);
    fd_led = open(path, O_RDWR);

    return 0;
}

/*
 * init buttonNb, return fd /value
 */
int open_button(const char* buttonNb)
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
  sprintf(path,"%s%s/edge",GPIO_GPIO,buttonNb);
  f = open(path, O_WRONLY);
  write(f, INTERRUPT_RISING, strlen(INTERRUPT_RISING));
  close(f);

  
  // open gpio value attribute
  sprintf(path,"%s%s/value",GPIO_GPIO,buttonNb);
  f = open(path, O_RDWR);
  
  return f;
}


void button_inc_freq_handler(void){
    printf("inc freq\n");
    incFreq();
}
void button_dec_freq_handler(void){
    printf("dec freq\n");
    decFreq();
}
void button_switch_mode_handler(void){
    printf("switch mode\n");
    switchMode();
}

