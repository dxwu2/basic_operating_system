#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "i8259.h"
#include "lib.h"
#include "paging.h"
#include "syscall.h"

// all from OSDEV
#define PIT_DATA        0x40
#define COMMAND_REG     0x43
#define SET_MODE        0x36

#define PIT_IRQ         0
#define COUNTER_LO      0x0B 
#define COUNTER_HI      0xE9

// the terminal that is currently executing its process (in the round robin). This is an index into scheduling_array
int scheduled_process;

// scheduling array - at most 3 processes running (per Aamir). These are the pids
int scheduling_array[3];

/*
s0 -> s1 -> s2
fish


s[2] c -> shell3
s[1] g
s[0] f

-1
4->0
5


shell shell shell ...

2
1
0

if(-1)

regular scheduling

- pid -> pcb 
- how to get c in s[2] to have its parent point to shell3 (term_id?)

*/

// initialize the PIT
void init_PIT(void);
