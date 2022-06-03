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
#include <stdio.h>
#include <string.h>


int main()
{

  // Allocates 50 x 1Megabyte
  char * ptr = malloc(50 * 1024*1024);
  if(ptr== NULL){
    perror("FAILED allocating 50M.\n");
  }

  memset(ptr, 0, 50*1024*1024);
  printf("Success in filling blocks with 0\n");

  free(ptr);
  return 0; 
}