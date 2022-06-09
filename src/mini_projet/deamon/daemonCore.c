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
 * Purpose:	core of the daemon, control, lie les éléments entre eux
 *
 * Autĥor:	Gaetan Kolly, Andrea Enrile
 * Date:	09.06.2022
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include "oledControl.h"

#include "daemonCore.h"

#define EPOLL_MAX_EVENT 10


ModeType mode=Auto;     // todo: don t store, read directy from sysfs
int freq=1;             // todo: don t store, read directy from sysfs

struct_epoll_arr* epoll_arr_ptr[NUMBER_MODE];
int epfd;

/*
* get/set Mode
*/
ModeType getMode(){
    //todo: sysfs

    return mode;
}
void setMode(ModeType mode_){
    mode=mode_;
    // todo sysfs
}

/*
* switch mode
*/
void switchMode(){
    ModeType mode = getMode();
    removeEpoll(mode,epfd);
    mode=!mode;
    setEpoll(mode,epfd);
    setMode(mode);
    displayMode(mode);
}

int getFreq(){
    // todo read in sysfs
    return freq;
}

void setFreq(int freq_){
    // todo sysfs
    freq=freq_;
}

/*
* increment actual freq
*/
void incFreq(){
    int freq=getFreq();
    freq++;
    setFreq(freq);
    displayFreq(freq);
}

/*
* decrement actual freq
*/
void decFreq(){
    int freq=getFreq();
    if(freq<=1){
        freq=0;
    }
    freq--;
    setFreq(freq);
    displayFreq(freq);
}

/*
* Manage epoll
*/
void setEpollPointer(int epfd_,struct_epoll_arr* epoll_arr_auto_,struct_epoll_arr* epoll_arr_man_){
    epfd=epfd_;
    epoll_arr_ptr[Auto]=epoll_arr_auto_;
    epoll_arr_ptr[Man]=epoll_arr_man_;
}

void removeEpoll(ModeType mode, int epfd){
    for(int i=0; i<epoll_arr_ptr[mode]->nbInEpoll;i++){
        int ret = epoll_ctl(epfd, EPOLL_CTL_DEL, 
                    epoll_arr_ptr[mode]->epoll_arr[i].fd, 
                    &epoll_arr_ptr[mode]->epoll_arr[i].event);
        if (ret ==-1)   
            perror("ERROR");
    }
}

void setEpoll(ModeType mode, int epfd){
    for(int i=0; i<epoll_arr_ptr[mode]->nbInEpoll;i++){
        int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, 
                    epoll_arr_ptr[mode]->epoll_arr[i].fd, 
                    &epoll_arr_ptr[mode]->epoll_arr[i].event);
        if (ret ==-1)   
            perror("ERROR");
    }
    // clear direct event, wait 1ms
    struct epoll_event events[10];
    epoll_wait(epfd, events, 10, 1); 
}

