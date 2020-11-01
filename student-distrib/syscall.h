#ifndef SYSCALL_H
#define SYSCALL_H

#include "lib.h"
#include "paging.h"

/* macros */
#define MAX_NUM_PIDS    6   // up to 8 open files per task, but one is stdin and one is stdout
#define UNUSED          0   // for pid_array
#define USED            1   // for pid_array

/* System call handler declaration */
void system_call();

/* Actual system call handlers (CP3) */
int32_t sys_halt (uint8_t status);
int32_t sys_execute (const uint8_t command);
int32_t sys_read (int32_t fd, void* buf, int32_t nbytes);
int32_t sys_write (int32_t fd, void* buf, int32_t nbytes);
int32_t sys_open (const uint8_t* filename);
int32_t sys_close (int32_t fd);
int32_t sys_getargs (uint8_t* buf, int32_t nbytes);
int32_t sys_vidmap (uint8_t** screen_start);

/* local variables */
static int pid_array[MAX_NUM_PIDS];

#endif
