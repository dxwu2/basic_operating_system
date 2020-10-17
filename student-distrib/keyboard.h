/* keyboard.h - Defines used for keyboard
 */

#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "i8259.h"

#define KEYBOARD_PORT 0x60      // per OSDEV
#define KEYBOARD_IRQ_LINE 0x1   // per wikipedia

// initialize keyboard for use
void keyboard_init(void);

// handler for keyboard
void keyboard_handler(void);

#endif /* _KEYBOARD_H */