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

//#include "lib/ssd1306.h"
#include "ssd1306.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "oledControl.h"

/*
* Init Oled write the standard messages
*/
int initOled(){
    
    ssd1306_init();

    ssd1306_set_position (0,0);
    ssd1306_puts("Fan regulation");
    ssd1306_set_position (0,1);
    ssd1306_puts("--------------");
    
    displayMode(Auto);
    displayTemp(0);
    displayFreq(0);

    return 0;
}

/*
* display mode
*/
void displayMode(ModeType mode){
    ssd1306_set_position (0,2);
    char line[50];
    if(mode==Auto){
        const char* str="Mode= Automatic";
        strcpy(line,str);
    }else{
        const char* str="Mode= Manual   ";
        strcpy(line,str);
    }
    ssd1306_puts(line);
}

/*
* display temp
*/
void displayTemp(float temp){
    ssd1306_set_position (0,3);
    char str[50];
    sprintf(str,"Temp: %.2f'C",temp);
    str[MAX_DIPLAY_CHAR_LINE+1]=0;
    ssd1306_puts(str);
}

/*
* display freq
*/
void displayFreq(int freq){
    ssd1306_set_position (0,4);
    char str[50];
    sprintf(str,"Freq: %dHz",freq);
    str[MAX_DIPLAY_CHAR_LINE+1]=0;
    ssd1306_puts(str);
}
