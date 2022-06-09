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
 * Purpose:	deamon for mini-projet
 *
 * Autĥor:	Gaetan Kolly, Andrea Enrile
 * Date:	09.06.2022
 */
#include <sys/epoll.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>


#include "oledControl.h"
#include "gpio.h"
#include "daemonCore.h"

#define EPOLL_MAX_EVENT 10


int main()
{
    /*
    * Initialisation 
    */
    initOled();

    /*
    * Initialisation GPIO
    */
    
    //init_led();
    
    struct_epoll_elem inc_button; 
    inc_button.fd= open_button(K1);
    inc_button.event.events=EPOLLET,
    inc_button.event.data.ptr = &button_inc_freq_handler;

    struct_epoll_elem dec_button; 
    dec_button.fd= open_button(K2);
    dec_button.event.events=EPOLLET,
    dec_button.event.data.ptr = &button_dec_freq_handler;
    
    struct_epoll_elem mode_button; 
    mode_button.fd= open_button(K3);
    mode_button.event.events=EPOLLET,
    mode_button.event.data.ptr = &button_switch_mode_handler;

    /*
    * Initialisation epoll
    */
    int epfd=epoll_create1(0);
    
    // auto 
    struct_epoll_arr epoll_auto;
    epoll_auto.nbInEpoll=1;     // dont forget to update number of element
    epoll_auto.epoll_arr = (struct_epoll_elem*) malloc(sizeof(struct_epoll_elem)*epoll_auto.nbInEpoll);
    epoll_auto.epoll_arr[0]=mode_button;

    //manual
    struct_epoll_arr epoll_man;
    epoll_man.nbInEpoll=3;     // dont forget to update number of element
    epoll_man.epoll_arr = (struct_epoll_elem*) malloc (sizeof(struct_epoll_elem)*epoll_man.nbInEpoll);
    epoll_man.epoll_arr[0]=mode_button;   
    epoll_man.epoll_arr[1]=inc_button;   
    epoll_man.epoll_arr[2]=dec_button;  

    // give epoll ref
    setEpollPointer(epfd,&epoll_auto,&epoll_man);
    setEpoll(Auto,epfd);

    /*
    * start app
    */
    while(1==1){
        struct epoll_event events[EPOLL_MAX_EVENT];

        int nr = epoll_wait(epfd, events, EPOLL_MAX_EVENT, -1);

        for(int i=0;i<nr;i++){
            void(*handler_fct)() = events[i].data.ptr;
			handler_fct();
        }
    }

    return 0;
}

