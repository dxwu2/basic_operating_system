/* keyboard.h - Defines used for keyboard
 */

#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "i8259.h"
#include "lib.h"

#define KEYBOARD_PORT 0x60      // per OSDEV
#define KEYBOARD_IRQ_LINE 0x1   // per wikipedia

#define KEYBOARD_BUF_SIZE 128   // per doc

// all scancodes per osdev
#define LSHIFT 0x2A
#define RSHIFT 0x36
#define LSHIFT_RELEASE 0xAA
#define RSHIFT_RELEASE 0xB6
#define CAPS_LOCK 0x3A
#define CTRL 0x1D
#define CTRL_RELEASE 0x9D
#define ALT 0x38
#define ALT_RELEASE 0xB8
#define ENTER 0x1C
#define L 0x26
#define BACKSPACE 0x0E

// for the keyboard buffer
char keyboard_buf[KEYBOARD_BUF_SIZE];     // need keyboard buffer -> size 128 char but last character is null '\0'
int buf_idx;                          // idx for current place in keyboard buffer -> initialize to 0

// key states
int shift_pressed;
int ctrl_pressed;
int caps_lock_pressed;
int alt_pressed;

// initialize keyboard for use
void keyboard_init(void);

// processes key pending on state
void process_key(uint8_t scancode);

// adds to current keyboard buffer
void add_to_buf(char letter);

// deletes most recent char from buffer
void delete_from_buf(void);

// clears buffer
void clear_keyboard_buf(void);

// returns keyboard buffer
void keyboard_return(void);

// handler for keyboard - make it extern for asm linkage
extern void keyboard_handler(void);

#endif /* _KEYBOARD_H */
