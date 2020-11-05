#ifndef SYSCALL_H
#define SYSCALL_H

#include "lib.h"
#include "paging.h"

/* macros */
#define MAX_NUM_PIDS    6   // up to 8 open files per task, but one is stdin and one is stdout
#define UNUSED          0   // for pid_array
#define USED            1   // for pid_array

#define MAX_CMD_LENGTH 127              // looking online, linux commands don't go too high
#define MAX_ARGS_LENGTH 127             // keyboard buffer at most 127 char, subtract 10 for command

#define FDA_SIZE        8   // 2 are for stdin and stdout, other 6 are for open files

/* System call handler declaration */
void system_call();

/* Actual system call handlers (CP3) */
int32_t sys_halt (uint8_t status);
int32_t sys_execute (const uint8_t* command);
int32_t sys_read (int32_t fd, void* buf, int32_t nbytes);
int32_t sys_write (int32_t fd, void* buf, int32_t nbytes);
int32_t sys_open (const uint8_t* filename);
int32_t sys_close (int32_t fd);
int32_t sys_getargs (uint8_t* buf, int32_t nbytes);
int32_t sys_vidmap (uint8_t** screen_start);

// function for invalid/nonexistent file operations
int32_t bad_call(void);

// initialzing a pcb for use
void init_pcb(pcb_t* curr_pcb, int pid, uint8_t* args);

typedef struct fops {
    // use function pointers since we have distinct use-cases for each (i.e. rtc_read vs terminal_read)

    int32_t (*open)(const uint8_t* filename);                   // open
    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);     // read
    int32_t (*write)(int32_t fd, void* buf, int32_t nbytes);    // write
    int32_t (*close)(int32_t fd);                               // close

} fops_t;

typedef struct files {
    fops_t fops_ptr;            // fops table ptr
    uint32_t inode;             // inode corresponding to the file
    uint32_t file_position;     // keeps track of where the user is currently reading from in the file. 
                                // Every read system all should update this member
    uint32_t flags;             // marking this file descriptor as "in-use" (check Appendix A)
} files_t;


// 4 - 8mb
// each user's kernel stack is 8kB
// base of each of these
// 8MB
// 8MB - (pid * 8kB)

// shut address -> street address -> struct address

// shengkun is OP :)
// FD table
// Process ID
// ptr parent pcb
// ptr child pcb
// entry point
// page addr
// old esp -> kernel?
// old ebp
// args     - later ch4

typedef struct pcb {
    files_t fda[FDA_SIZE];              // file descriptor table/array
    uint32_t base_kernel_stack;         // saves the base of each 8kB kernel stack for each user program
    pcb_t* parent_pcb;                  // the parent of this process (which file called the current one)
    pcb_t* child_pcb;                   // if this file needs to call another file
    uint32_t* entry_position;

    int old_esp;                        // esp of the parent process
    int old_ebp;                        // ebp of the parent process

    uint8_t* args;                      // for getargs syscall 

} pcb_t;


/* local variables */
static int pid_array[MAX_NUM_PIDS];

#endif
