#include "syscall.h"

/* void handle_system_call()
 * Temporary system call handler 
 * Inputs: None
 * Outputs: None
 * Side Effects: Prints "System call was called."
 */
void handle_system_call(){
    clear();
    printf("System call (vector 0x80) was called.\n");
    while(1);
}

/* set up structures and local variables used in passing in parameters for system calls */
void syscall_init() {
    int i;
    for (i = 0; i < MAX_NUM_PIDS; i++) {
        pid_array[i] = UNUSED;
    }
}

int32_t sys_halt (uint8_t status){
    // REMEMBER TO SET PID TO UNUSED
    return 0;
}

int32_t sys_execute (const uint8_t command){
    // parse the command - label where the function we're calling is (makes it easier to explain)

    // check if executable

    /* step 3 - paging */
    // find an unused pid (0-5)
    int pid;
    for (pid = 0; pid < MAX_NUM_PIDS; pid++) {
        if (pid_array[pid] == UNUSED) {
            pid_array[pid] = USED;
            break;
        }
        // if pid_array is full
        if (pid == MAX_NUM_PIDS - 1) {
            return -1;
        }
    }
    // map user program
    map_user_program(pid);

    // user level program loader

    return 0;
}

int32_t sys_read (int32_t fd, void* buf, int32_t nbytes){
    return 0;
}

int32_t sys_write (int32_t fd, void* buf, int32_t nbytes){
    return 0;
}

int32_t sys_open (const uint8_t* filename){
    return 0;
}

int32_t sys_close (int32_t fd){
    return 0;
}

int32_t sys_getargs (uint8_t* buf, int32_t nbytes){
    return 0;
}

int32_t sys_vidmap (uint8_t** screen_start){
    return 0;
}
