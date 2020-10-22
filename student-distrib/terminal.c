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
    i = 0;

    // keep reading until newline
    while(keyboard_buf[i] != '\n'){
        ((char*)buf)[i] = keyboard_buf[i];      // read from keyboard buffer TO buf
        i++;                                    // increment index
    }

    ((char*)buf)[i] = keyboard_buf[i];          // line should include the LF character per doc

    clear_keyboard_buf();                       // need to clear the buffer at the end
    return nbytes;
}


/* terminal_write(int32_t fd, const void* buf, int32_t nbytes)
 * Inputs: fd - not relevant to terminal
 *          buf - data to write
 * Return Value: int32_t (number of bytes written on success, -1 on failure?)
 * Function: Writes n bytes from buf to screen */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes){
    int i;      // for looping

    for(i = 0; i < nbytes; i++){
        char letter = ((char*)buf)[i];      // converting void pointer to char pointer -> need to index by array (ea char = 1 byte)
        putc(letter);
    }

    return nbytes;
}

