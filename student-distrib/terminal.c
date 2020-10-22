/* terminal.c - Functions for terminal driver
 */

#include "terminal.h"

/* terminal_open(const uint8_t* command)
 * Inputs: filename - not relevant to terminal
 * Return Value: int32_t (0 on success, -1 on failure)
 * Function: not relevant to terminal */
int32_t terminal_open(const uint8_t* filename){
    return 0;
}


/* terminal_close(int32_t fd)
 * Inputs: fd - not relevant to terminal
 * Return Value: int32_t (0 on success, -1 on failure)
 * Function: not relevant to terminal */
int32_t terminal_close(int32_t fd){
    return 0;
}


/* terminal_read(int32_t fd, void* buf, int32_t nbytes)
 * Inputs: fd - not relevant to terminal
 *          buf - data to read to
 *          nbytes - number of bytes to read
 * Return Value: int32_t (number of bytes written on success, -1 on failure?)
 * Function: Reads from keyboard buffer to buf until a newline '\n' */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes){
    int i;      // for looping, start at 0

    if(nbytes <= 0 || buf == NULL){
        return -1;
    }

    // if keep waiting if no key was pressed OR buf_idx reached its limit
    while(!flag && buf_idx < KEYBOARD_BUF_SIZE - 1);

    for(i = 0; (i < nbytes) && (i < KEYBOARD_BUF_SIZE); i++){
        ((char*)buf)[i] = keyboard_buf[i];      // read from keyboard buffer TO buf
        if(keyboard_buf[i] == '\n'){
            flag = 0;
            break;
        }
    }

    clear_keyboard_buf();       // need to clear the buffer at the end
    return i+1;                 // return size of buffer
}


/* terminal_write(int32_t fd, const void* buf, int32_t nbytes)
 * Inputs: fd - not relevant to terminal
 *          buf - data to write
 *          nbytes - number of bytes
 * Return Value: int32_t (number of bytes written on success, -1 on failure?)
 * Function: Writes n bytes from buf to screen */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes){
    int i;      // for looping

    if(nbytes <= 0 || buf == NULL){
        return -1;
    }

    for(i = 0; i < nbytes; i++){
        char letter = ((char*)buf)[i];      // converting void pointer to char pointer -> need to index by array (ea char = 1 byte)
        putc(letter);
    }

    return nbytes;
}

