/* keyboard.c - Functions to read input from keyboard and print to screen
 */

#include "keyboard.h"
#include "types.h"
#include "lib.h"

// making a map of scan code set 2 (https://wiki.osdev.org/PS/2_Keyboard)
// tedious as shit
// not worrying about keyboard, so just go up to 0x50 range
static char nornal_map[6][15] = {
    // 0    1     2    3    4    5    6    7     8    9    10    11    12     13   14
    {'\0', '1', '\0', '1', '1', '1', '1', '1', '\0', '1', '1', '1', '1', '1', '1'},   // 0x0_ are the F_ keys (1 for now)
    {'\0', '\0', '\0', '\0', '\0', 'q', '1', '\0', '\0', '\0', 'z', 's', 'a', 'w', '2'},    // 0x1_
    {'\0', 'c', 'x', 'd', 'e', '4', '3', '\0', '\0', ' ', 'v', 'f', 't', 'r', 'e'},     // 0x2_
    {'\0', 'n', 'b', 'h', 'g', 'y', '6', '\0', '\0', '\0', 'm', 'j', 'u', '7', '8'},    // 0x3_
    {'\0', ',', 'k', 'i', 'o', '0', '9', '\0', '\0', '.', '/', 'l', ';', 'p', '-'},   // 0x4_
    {'\0', '\0', 39, '\0', '[', '=', '\0', '\0', '\0', '\0', '\0', ']', '\0', 92, '\0'}   // 0x5_ ; 39 is ' ; 92 is '\'
}

/*
 * keyboard_init
 *   DESCRIPTION: Initializes the keyboard for use
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: enables IRQ1 on master pic
 */   
void keyboard_init(void){
    // enable IRQ1 on master PIC
    enable_irq(KEYBOARD_IRQ_LINE);
}


/*
 * keyboard_handler
 *   DESCRIPTION: Figures out what key was pressed and processes it (after being called) 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: ?
 */   
void keyboard_handler(void){
    uint8_t scancode;

    // press key -> generates interrupt -> calls handler (THIS)
    //  -> figure out what key was pressed and process it (print to screen)

    // first need to read data to get scancode
    scancode = inb(KEYBOARD_PORT);




}
