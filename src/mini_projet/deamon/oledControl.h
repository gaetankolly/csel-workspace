#pragma once
#ifndef OLEDCONTROL_H
#define OLEDCONTROL_H

#include "daemonCore.h"

#define MAX_DIPLAY_CHAR_LINE 16

int initOled();
void displayMode(ModeType mode);
void displayTemp(float temp);
void displayFreq(int freq);

#endif