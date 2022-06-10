#pragma once
#ifndef GPIO_H
#define GPIO_H

#define K1 "0"
#define K2 "2"
#define K3 "3"

int open_button(const char* buttonNb);

void button_inc_freq_handler(int fd);
void button_dec_freq_handler(int fd);
void button_switch_mode_handler(int fd);


#endif