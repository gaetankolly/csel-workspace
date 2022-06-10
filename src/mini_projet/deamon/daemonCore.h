#pragma once
#ifndef DAEMONCORE_H
#define DAEMONCORE_H

#include <sys/epoll.h>

// mode
#define NUMBER_MODE 2
typedef enum{Auto=0,Man=1} ModeType;

ModeType getMode();
void setMode(ModeType mode);
void switchMode();

// freq
int getFreq();
void setFreq(int freq);
void incFreq();
void decFreq();

// epoll 
typedef struct epoll_element{
    int fd;
    struct epoll_event event;
}struct_epoll_elem;

// array of epoll element
typedef struct epoll_arr{
    int nbInEpoll;
    struct_epoll_elem* epoll_arr;
}struct_epoll_arr;

// epoll data
typedef struct epoll_data_custom{
    int fd;
    void* handler;            //void (*handler)(int)
}struct_epoll_data;

void setEpollPointer(int epfd_,struct_epoll_arr* epoll_arr_auto_,struct_epoll_arr* epoll_arr_man_);
void removeEpoll(ModeType mode, int epfd);
void setEpoll(ModeType mode, int epfd);

#endif