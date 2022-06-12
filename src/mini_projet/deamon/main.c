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
#include "socket.h"

#define EPOLL_MAX_EVENT 10


int main()
{
    /*
    * Initialisation led
    */
    initOled();

    /*
    * Initialisation GPIO
    */
    init_led();
    
    struct_epoll_elem inc_button; 
    inc_button.fd= open_button(K1);
    inc_button.event.events=EPOLLET;
    struct_epoll_data data_inc_button={
        .fd=inc_button.fd,
        .handler =&button_inc_freq_handler
    };
    inc_button.event.data.ptr = &data_inc_button;

    struct_epoll_elem dec_button; 
    dec_button.fd= open_button(K2);
    dec_button.event.events=EPOLLET;
    struct_epoll_data data_dec_button={
        .fd=dec_button.fd,
        .handler =&button_dec_freq_handler
    };
    dec_button.event.data.ptr = &data_dec_button;

    struct_epoll_elem mode_button; 
    mode_button.fd= open_button(K3);
    mode_button.event.events=EPOLLET;
    struct_epoll_data data_mode_button={
        .fd=mode_button.fd,
        .handler =&button_switch_mode_handler
    };
    mode_button.event.data.ptr = &data_mode_button;

    /*
    * init socket
    */
    struct_epoll_elem socket_ep; 
    socket_ep.fd= create_socket();
    socket_ep.event.events=EPOLLET|EPOLLIN;
    struct_epoll_data data_socket={
        .fd=socket_ep.fd,
        .handler =&newConnection_handler
    };
    socket_ep.event.data.ptr = &data_socket;

    /*
    * sys fs
    */
    struct_epoll_elem sysTemp_ep; 
    sysTemp_ep.fd=openTempSys();
    sysTemp_ep.event.events=EPOLLIN;
    struct_epoll_data data_sysTemp={
        .fd=sysTemp_ep.fd,
        .handler =&tempSys_handler
    };
    sysTemp_ep.event.data.ptr = &data_sysTemp;

    struct_epoll_elem freqSys_ep; 
    freqSys_ep.fd=openFreqSys();
    freqSys_ep.event.events=EPOLLIN;
    struct_epoll_data data_freqSys={
        .fd=freqSys_ep.fd,
        .handler =&freqSys_handler
    };
    freqSys_ep.event.data.ptr = &data_freqSys;

    /*
    * Initialisation epoll
    */
    int epfd=epoll_create1(0);
    
    // auto 
    struct_epoll_arr epoll_auto;
    epoll_auto.nbInEpoll=4;     // dont forget to update number of element
    epoll_auto.epoll_arr = (struct_epoll_elem*) malloc(sizeof(struct_epoll_elem)*epoll_auto.nbInEpoll);
    epoll_auto.epoll_arr[0]=mode_button;
    epoll_auto.epoll_arr[1]=socket_ep;
    epoll_auto.epoll_arr[2]=sysTemp_ep;
    epoll_auto.epoll_arr[3]=freqSys_ep;
    

    //manual
    struct_epoll_arr epoll_man;
    epoll_man.nbInEpoll=5;     // dont forget to update number of element
    epoll_man.epoll_arr = (struct_epoll_elem*) malloc (sizeof(struct_epoll_elem)*epoll_man.nbInEpoll);
    epoll_man.epoll_arr[0]=mode_button;   
    epoll_man.epoll_arr[1]=inc_button;   
    epoll_man.epoll_arr[2]=dec_button;  
    epoll_man.epoll_arr[3]=socket_ep;
    epoll_man.epoll_arr[4]=sysTemp_ep;

    // give epoll ref
    setEpollPointer(epfd,&epoll_auto,&epoll_man);
    ModeType mode=getMode();
    setEpoll(mode,epfd);
    displayMode(mode);

    /*
    * start app
    */
    while(1==1){
        struct epoll_event events[EPOLL_MAX_EVENT];
        
        int nr = epoll_wait(epfd, events, EPOLL_MAX_EVENT, -1);

        for(int i=0;i<nr;i++){
            struct_epoll_data* epoll_data = (struct_epoll_data*) events[i].data.ptr;
            void (*handler_fct)(int) = epoll_data->handler;
			handler_fct(epoll_data->fd);
        }
    }

    return 0;
}

