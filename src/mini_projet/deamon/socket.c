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
 * Abstract: socket
 *
 * Purpose:	control IPC communication with socket
 *
 * Autĥor:	Gaetan Kolly, Andrea Enrile
 * Date:	09.06.2022
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "daemonCore.h"
#include "oledControl.h"

#include "socket.h"

#define SOCKET_PATH "/tmp/socketDaemon"
#define MAX_BYTE_NUMBER 100

//int fd_socket;
struct sockaddr_un name;

int create_socket(){

    int fd_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd_socket < 0) {
        perror("Error socket");
    }

    memset(&name, 0, sizeof(name));
    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, SOCKET_PATH, sizeof(name.sun_path) - 1);

    if (bind(fd_socket, (const struct sockaddr *) &name, sizeof(name))<0) {
        perror("Error binding");
    }

    if(listen(fd_socket, 2)<0){
        perror("Socket listen");
    }

    return fd_socket;
}

void newConnection_handler(int fd_socket){
    //printf("new socket connection %d\n",fd_socket);
    
    int conn_fd = accept(fd_socket, NULL, NULL);
    if(conn_fd<0){
        perror("Accept failed");
        return;
    }
    //printf("Connection accepted\n");

    char readData[MAX_BYTE_NUMBER]={0};
    int nb=read(conn_fd,readData,MAX_BYTE_NUMBER-1);
    readData[nb]=0;
    
    //if(nb>0)
    //    printf("Recieved: %s\n",readData);

    char answer[MAX_BYTE_NUMBER]={0};
    if(process_cmd(readData,answer)<0){
        strncpy(answer,"Wrong commands",MAX_BYTE_NUMBER);
    }

    write(conn_fd,answer,strlen(answer));
    
    close(conn_fd);
    
}

void close_socket(){

}

// user cmd
int process_cmd(const char* input, char* answer){

    char cmd_tmp[MAX_BYTE_NUMBER]={0};
    strncpy(cmd_tmp,input,MAX_BYTE_NUMBER-1);
    char* cmd=strtok(cmd_tmp,";");
    char* arg=strtok(NULL,";");

    if(strncmp(cmd,"switchMode",MAX_BYTE_NUMBER)==0){
        switchMode();
        ModeType mode = getMode();
        if(mode==Auto){
            strncpy(answer,"Mode is set automatic",MAX_BYTE_NUMBER);
        }
        else{
            strncpy(answer,"Mode is set manual",MAX_BYTE_NUMBER);
        }
    }
    else if(strncmp(cmd,"inc",MAX_BYTE_NUMBER)==0){
        ModeType mode = getMode();
        if(mode==Auto){
            strncpy(answer,"Unable to inc in auto mode",MAX_BYTE_NUMBER);
        }
        else{
            incFreq();
            int freq=getFreq();
            sprintf(answer,"Freq= %d",freq);
        }
    }
    else if(strncmp(cmd,"dec",MAX_BYTE_NUMBER)==0){
        ModeType mode = getMode();
        if(mode==Auto){
            strncpy(answer,"Unable to dec in auto mode",MAX_BYTE_NUMBER);
        }
        else{
            decFreq();
            int freq=getFreq();
            sprintf(answer,"Freq= %d",freq);
        }
    }
    else if(strncmp(cmd,"getFreq",MAX_BYTE_NUMBER)==0){
        int freq = getFreq();
        sprintf(answer,"Freq= %d",freq);
    }
    else if(strncmp(cmd,"setFreq",MAX_BYTE_NUMBER)==0){
        printf("%s,%s",cmd,arg);
        ModeType mode = getMode();
        if(mode==Man){
            int freq=atoi(arg);
            setFreq(freq);
            freq=getFreq();
            sprintf(answer,"Freq= %d",freq);
            displayFreq(freq);
        }
        else{
            strncpy(answer,"Impossible to set freq",MAX_BYTE_NUMBER-1);
        }
    }
    else{
        return -1;
    }

    return 0;
}