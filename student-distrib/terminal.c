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
    int bytes;

    if(nbytes <= 0 || buf == NULL){
        return -1;
    }

    // wait until '\n' is pressed
    while(!key_flag);

    // bytes should be the minimum between keyboard buffer length and nbytes to read
    bytes = (strlen(keyboard_buf) < nbytes) ? strlen(keyboard_buf) : nbytes;

    // use memcpy from lib.c to copy from keyboard_buf to buf
    memcpy(buf, keyboard_buf, bytes);

    clear_keyboard_buf();       // need to clear the buffer at the end
    key_flag = 0;                   // reset key_flag to prevent reading
    return bytes;               // return size of buffer
}


/* terminal_write(int32_t fd, const void* buf, int32_t nbytes)
 * Inputs: fd - not relevant to terminal
 *          buf - data to write
 *          nbytes - number of bytes
 * Return Value: int32_t (number of bytes written on success, -1 on failure?)
 * Function: Writes n bytes from buf to screen */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes){
    int i;      // for looping
    int bytes;

    if(nbytes <= 0 || buf == NULL){
        return -1;
    }

    // bytes should be the minimum between keyboard buffer length and nbytes to read
    bytes = (strlen((char*)buf) < nbytes) ? strlen((char*)buf) : nbytes;

    for(i = 0; i < nbytes; i++){
        char letter = ((char*)buf)[i];      // converting void pointer to char pointer -> need to index by array (ea char = 1 byte)
        if(letter != '\0'){                 // do not print null bytes to screen
            putc(letter);
        }
    }

    return nbytes;
}


/* void switch_terminals(int next_term)
 * Inputs: int next_term
 *          buf - data to write
 *          nbytes - number of bytes
 * Return Value: int32_t (number of bytes written on success, -1 on failure?)
 * Function: Writes n bytes from buf to screen */
void switch_terminals(int next_term) {
    if (curr_term == next_term) return;


    // save vidmem into curr mem
    memcpy(terminals[curr_term].vidmem, (uint32_t*)VIDMEM_ADDRESS, FOUR_KB);
    
    // restore next state into vidmem
    memcpy((uint32_t*)VIDMEM_ADDRESS, terminals[next_term].vidmem, FOUR_KB);

    //remap vidmem to visible region
    map_vidmem();

    /*Save curr keyboard buf and cursor position to terminal struct*/
    memcpy(terminals[curr_term].keyboard_buf, keyboard_buf, KEYBOARD_BUF_SIZE);
    // save current screen x/y into current term
    terminals[curr_term].term_x = screen_x;
    terminals[curr_term].term_y = screen_y;
    
    // restore next screen x/y and keyboard buf
    screen_x = terminals[next_term].term_x;
    screen_y = terminals[next_term].term_y;
    // keyboard_buf = terminals[next_term].keyboard_buf;
    memcpy(keyboard_buf, terminals[next_term].keyboard_buf, KEYBOARD_BUF_SIZE);
    
    // Update curr_term to reflect terminal switch
    curr_term = next_term;
}
