#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define ADRESS 0x01c14200
#define PAGE_ADDRESS (ADRESS&0xfffff000) 
#define OFFSET ((ADRESS&0x00000fff)>>2) 
#define MEM_STOP 0x01c1420c

int main(){

    /* Exo */
    printf("Personal solution\n");
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
                fd,                 // descripteur du fichier correspondant au pilote
                PAGE_ADDRESS        // offset des registres en mémoire
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

    /* Solution */
    printf("Teacher solution\n");
     /* open memory file descriptor */
    fd = open("/dev/mem", O_RDWR);
    if (fd < 0) {
        printf("Could not open /dev/mem: error=%i\n", fd);
        return -1;
    }

    size_t psz     = getpagesize();
    off_t dev_addr = 0x01c14200;
    off_t ofs      = dev_addr % psz;
    off_t offset   = dev_addr - ofs;
    printf(
        "psz=%lx, addr=%lx, offset=%lx, ofs=%lx\n", psz, dev_addr, offset, ofs);

    /* map to user space nanopi internal registers */
    volatile uint32_t* regs =
        mmap(0, psz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);

    if (regs == MAP_FAILED)  // (void *)-1
    {
        printf("mmap failed, error: %i:%s \n", errno, strerror(errno));
        return -1;
    }

    uint32_t chipid[4] = {
        [0] = *(regs + (ofs + 0x00) / sizeof(uint32_t)),
        [1] = *(regs + (ofs + 0x04) / sizeof(uint32_t)),
        [2] = *(regs + (ofs + 0x08) / sizeof(uint32_t)),
        [3] = *(regs + (ofs + 0x0c) / sizeof(uint32_t)),
    };

    printf("NanoPi NEO Plus2 chipid=%08x'%08x'%08x'%08x\n",
           chipid[0],
           chipid[1],
           chipid[2],
           chipid[3]);

    munmap((void*)regs, psz);
    close(fd);

    return 0;
}
