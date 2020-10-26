/* terminal.h - Defines used for terminal.c
 */

#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "keyboard.h"
#include "types.h"
#include "lib.h"

// terminal driver functions below

// terminal open function
int32_t terminal_open(const uint8_t* filename);

// terminal close function
int32_t terminal_close(int32_t fd);

// terminal read function
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);

// terminal write function
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);


#endif /* _TERMINAL_H */
