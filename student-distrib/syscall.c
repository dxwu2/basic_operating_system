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
