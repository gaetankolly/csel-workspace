#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define ADRESS 0x01c14200
#define PAGE_ADDRESS (ADRESS&0xfffff000) 
#define OFFSET ((ADRESS&0x00000fff)>>2) 
#define MEM_STOP 0x01c1420c

int main(){


    int fd = open ("/dev/mem",O_RDWR,0);
    if(fd<0){
        printf("file not found\n");
        return -1;
    }

    unsigned int* memMap =0;
    unsigned int chipID[4];

    memMap= (unsigned int*) mmap (      
                NULL,               // généralement NULL, adresse de départ en mémoire virtuelle
                0x1000,              //taille de la zone à placer en mémoire virtuelle
                PROT_READ,          // droits d’accès à la mémoire: read, write, execute
                MAP_SHARED,         // visibilité de la page pour d’autres processus: shared, private
                fd,                     // descripteur du fichier correspondant au pilote
                ADRESS&0xfffff000               // offset des registres en mémoire
                );

    if(memMap ==NULL){
        close(fd);
        return -1;
    }

    printf("Chip ID: \n");
    for(int i=0; i<4; i++){ 
        //printf("%d\n", memMap[i]);
        chipID[i]=(*(memMap+(OFFSET)+i));
        printf("%x\n", chipID[i]);
    }
    printf("\n");
    

    close(fd);

}
