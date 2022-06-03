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
 * Purpose:	test cgroup
 *
 * Autĥor:	Gaetan Kolly, Andrea Enrile
 * Date:	25.mai.2022
 */
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>

int main(){

 // forking
  pid_t pid = fork();
  if (pid == 0){
    // child
    __uint32_t a=0;
    while (1==1){
      a++;
      if(a==pow(2,32)-1){
        printf("child filled an uint\n");
      }
    }

    exit(0);
  } 
  else if (pid > 0){
    // parent
    __uint32_t a=0;
    while (1==1){
      a++;
      if(a==pow(2,32)-1){
        printf("parent filled an uint\n");
      }
    }
    // wait child to finishes
    int status = 0;
    waitpid(pid, &status, 0);
  }
  else{
    perror("Error fork");
  }


}