/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Implementation of read and write using the developped
 *                   module mymodule.
 * @author         : Andrea Enrile & Gaëtan Kolly
 * @creation date  : 07.04.2021
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* Private variables ---------------------------------------------------------*/
static const char *text1 =
  "\n"
  "Sometimes we thing that by writing....\n"
  "a single text will be enough, however....\n"
  "it's really complicated explaining everything....\n"
  "in only one breef text.\n";

static const char *text2 =
  "\n"
  "So we will use this second text to tell you that the....\n"
  "Answer to the Ultimate Question of Life, the Universe, and Everything....\n"
  "is simply 42!!!!!\n";

static const char *filler =
  "we dont know what to say anymore >_<\n";

/* User code -----------------------------------------------------------------*/
  int main(int argc, char *argv[]){

    int fdw = 0; // file pointer for writing
    int fdr = 0; //file pointer for reading
    int s;
    int isNotEmpty = 1;

    if (argc <= 1){
      printf("No argument specified");
      return 0;
    }

    // writing part of the code
    fdw = open(argv[1], O_RDWR);
    if (fdw==-1){
      printf("Failed opening file %s for writing\n", argv[1]);
      return -1;
    }
    
    write(fdw, text1, strlen(text1));
    write(fdw, text2, strlen(text2));

    do{
      s = write(fdw, filler, strlen(filler));
    } while (s > 0);
    close(fdw);

    // reading part of the code
    fdr = open(argv[1], O_RDONLY);
    if (fdr==-1){
      printf("Failed opening file %s for reading\n", argv[1]);
      return -1;
    }

    while (isNotEmpty){
      char buff[100];
      ssize_t sz = read(fdr, buff, sizeof(buff) - 1);
      if (sz <= 0)
        isNotEmpty = 0;
      buff[sizeof(buff) - 1] = 0;
      printf("%s", buff);
    }
    close(fdr);

    return 0;
  }

/**********************************END OF FILE*********************************/
