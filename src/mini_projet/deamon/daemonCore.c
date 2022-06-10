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
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>

#include "oledControl.h"

#include "daemonCore.h"

#define EPOLL_MAX_EVENT 10
#define PATH_SYS_TEMP "/sys/bus/platform/devices/FanControllerDriver/temp"
#define PATH_SYS_FREQ "/sys/bus/platform/devices/FanControllerDriver/pwmFreq"
#define PATH_SYS_MODE "/sys/bus/platform/devices/FanControllerDriver/mode"
#define FREQ_MAX (20)


//ModeType mode=Auto;     // todo: don t store, read directy from sysfs
//int freq=1;             // todo: don t store, read directy from sysfs

struct_epoll_arr* epoll_arr_ptr[NUMBER_MODE];
int epfd;

int fd_sys_freq;

/*
* get/set Mode
*/
ModeType getMode(){
    char readData[10]={0};
    int fd=open(PATH_SYS_MODE,O_RDONLY);
    if(fd<0)
        perror("error sys mode");
    
    int nb=read(fd,readData,9);
    readData[nb]=0;
    close(fd);
    if(readData[0]=='0'){
        return Auto;
    }
    else if(readData[0]=='1'){
        return Man;
    }
    else{
        perror("Error reading mode");
        return Auto;
    }
}
void setMode(ModeType mode){
    int fd=open(PATH_SYS_MODE,O_WRONLY);
    if(fd<0)
        perror("error sys mode");
    
    
    if(mode == Auto){
        write(fd,"0",1);
    }
    else if(mode== Man){
        write(fd,"1",1);
    }
    else{
        perror("you are stupid");
    }
    close(fd);
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
    char readData[10]={0};
    lseek(fd_sys_freq,0,SEEK_SET);
    int nb=read(fd_sys_freq,readData,9);
    readData[nb]=0;
    int freq=atoi(readData);
    return freq;
}

void setFreq(int freq){
    char writeData[20]={0};
    snprintf(writeData,19,"%d",freq);  
    lseek(fd_sys_freq,0,SEEK_SET);  
    write(fd_sys_freq,writeData,20);
}


/*
* increment actual freq
*/
void incFreq(){
    int freq=getFreq();
    if(freq<FREQ_MAX){
        freq++;
    }
    else{
        freq=FREQ_MAX;
    }
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
    else{
        freq--;
    }
    setFreq(freq);
    displayFreq(freq);
}

/* 
* SYS fs
*/
int openTempSys(){
    int fd=open(PATH_SYS_TEMP, O_RDONLY);
    if(fd<0){
        perror("Error opening sys temp");
    }

    return fd;
}

void tempSys_handler(int fd){

    char readData[10]={0};
    lseek(fd,0,SEEK_SET);
    int nb=read(fd,readData,9);
    readData[nb]=0;
    float temp=(float)atof(readData);
    temp/=1000;
    displayTemp(temp);
}

int openFreqSys(){
    fd_sys_freq=open(PATH_SYS_FREQ, O_RDWR);
    if(fd_sys_freq<0){
        perror("Error opening sys Freq");
    }

    return fd_sys_freq;
}

void freqSys_handler(int fd){
    int freq= getFreq();
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

