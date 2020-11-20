#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "i8259.h"

#define PIT_DATA        0x40
#define COMMAND_REG     0x43
#define SET_MODE        0x36

#define PIT_IRQ         0
#define COUNTER_LO      0x0B 
#define COUNTER_HI      0xE9

// output freq * counter


void init_PIT(void);