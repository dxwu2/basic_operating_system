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

int32_t sys_halt (uint8_t status){
    return 0;
}

int32_t sys_execute (const uint8_t command){
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