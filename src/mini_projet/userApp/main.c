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
 * Abstract: user app fan control
 *
 * Purpose:	user app to communication with daemon
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

#define SOCKET_PATH "/socketFile"
#define MAX_BYTE_NUMBER 100

int main(int argc,char* argv[])
{
    char writeData[MAX_BYTE_NUMBER]={0};

    if(argc==1){
        printf("No commands entered\n");
        exit(EXIT_FAILURE);
    }
    else if(argc>=2)
    {
        strncpy(writeData,argv[1],MAX_BYTE_NUMBER-1);
    }
    else
    {
        printf("To many argument\n");
        exit(EXIT_FAILURE);
    }
    

    struct sockaddr_un name;

    int fdsocket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fdsocket <0) {
        perror("error creating socket");
        exit(EXIT_FAILURE);
    }

    memset(&name, 0, sizeof(name));
    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, SOCKET_PATH, sizeof(name.sun_path) - 1);

    if(connect(fdsocket, (const struct sockaddr *) &name,sizeof(name))<0){
        perror("Impossible to connect to daemon");
        exit(EXIT_FAILURE);
    }

    printf("success\n");

    if(write(fdsocket,writeData,strlen(writeData))<(ssize_t)strlen(writeData)){
        perror("Error writing into socket\n");
    }

    char readData[MAX_BYTE_NUMBER]={0};
    int nb=read(fdsocket,readData,MAX_BYTE_NUMBER-1);
    readData[nb]=0;  

    printf("%s\n",readData);

    close(fdsocket);

    exit(EXIT_SUCCESS);

    return 0;
}

