/* keyboard.c - Functions to read input from keyboard and print to screen
 */

#include "keyboard.h"

// making a map of scan code set 1 (https://wiki.osdev.org/PS/2_Keyboard)
// scan code set 1 - not worrying about keypad or F keys
static char normal_map[4][16] = {
    // 0    1     2    3   4     5     6   7    8    9    10   11   12   13   14   15
    {'\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0', '\0'},   // 0x0_
    {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\0', '\0', 'a', 's'},    // 0x1_
    {'d', 'f', 'g', 'h', 'j', 'k', 'l', ';', 39, '`', '\0', 92, 'z', 'x', 'c', 'v'},     // 0x2_ ; 39 is ' ; 92 is '\'
    {'b', 'n', 'm', ',', '.', '/', '\0', '\0', '\0', ' ', '\0', '\0', '\0', '\0', '\0', '\0'}    // 0x3_
};


// will have to create separate mapping for shift pressed / caps lock fuck

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
 *   SIDE EFFECTS: reads scancode from keyboard port
 */   
void keyboard_handler(void){
    // printf("called keyboard handler\n");
    cli();

    uint8_t scancode;
    uint8_t idx1;
    uint8_t idx2;
    char letter;

    // press key -> generates interrupt -> calls handler (THIS)
    //  -> figure out what key was pressed and process it (print to screen)

    scancode = inb(KEYBOARD_PORT);
    // printf("%d\n", scancode);

    // OOB check
    if(scancode > 0xD8){
        printf("Key was pressed");
    }
    // determine scan code if pressed / not released
    // 0x2D X pressed, 0xAD X released -> diff of 0x80 -> and 0x80!
    if(!(scancode & 0x80)){
        idx1 = (scancode & 0xF0) >> 4;      // grab high 4 bits to determine group, rshift by 4 to get raw number
        idx2 = (scancode & 0x0F);           // grab low 4 bits to determine specific char
        // placeholder test for now, not looking at indices > 3
        if(idx1 > 3){
            printf("Key was pressed");
        }
        else{
            letter = normal_map[idx1][idx2];
            printf("%c", letter);
        }
    }

    // at the end send an EOI
    send_eoi(KEYBOARD_IRQ_LINE);
    sti();
}
