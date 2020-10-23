/* keyboard.c - Functions to read input from keyboard and print to screen
 */

#include "keyboard.h"

// making a map of scan code set 1 (https://wiki.osdev.org/PS/2_Keyboard)
// scan code set 1 - not worrying about keypad or F keys
// 4x16 because we go up to 4 0x3_, and for each 0xi_ we look at most 16 characters
static char normal_map[4][16] = {
    // 0    1     2    3   4     5     6   7    8    9    10   11   12   13   14   15
    {'\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\0', '\0'},   // 0x0_
    {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', '\0', 'a', 's'},    // 0x1_
    {'d', 'f', 'g', 'h', 'j', 'k', 'l', ';', 39, '`', '\0', 92, 'z', 'x', 'c', 'v'},     // 0x2_ ; 39 is ' ; 92 is '\'
    {'b', 'n', 'm', ',', '.', '/', '\0', '\0', '\0', ' ', '\0', '\0', '\0', '\0', '\0', '\0'}    // 0x3_
};

static char shift_map[4][16] = {
    // 0    1     2    3   4     5     6   7    8    9    10   11   12   13   14   15
    {'\0', '\0', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\0', '\0'},   // 0x0_
    {'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', '\0', 'A', 'S'},    // 0x1_
    {'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', 34, '~', '\0', '|', 'Z', 'X', 'C', 'V'},     // 0x2_ ; 34 is "
    {'B', 'N', 'M', '<', '>', '?', '\0', '\0', '\0', ' ', '\0', '\0', '\0', '\0', '\0', '\0'}    // 0x3_
};

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

    buf_idx = 0;                    // have buf_idx start at beginning
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
    cli();      // begin crit section

    // press key -> generates interrupt -> calls handler (THIS)
    //  -> figure out what key was pressed and process it (print to screen)

    uint8_t scancode;
    scancode = inb(KEYBOARD_PORT);

    // OOB check
    if(scancode > 0xD8){
        return;     // for now do nothing
    }

    // determine if key/toggle state
    switch (scancode)
    {
    case CTRL:
        ctrl_pressed = 1;
        break;
    case CTRL_RELEASE:
        ctrl_pressed = 0;
        break;
    // both will press/release shift
    case LSHIFT:
    case RSHIFT:
        shift_pressed = 1;
        break;
    case LSHIFT_RELEASE:
    case RSHIFT_RELEASE:
        shift_pressed = 0;
        break;
    case ALT:
        alt_pressed = 1;
        break;
    case ALT_RELEASE:
        alt_pressed = 0;
        break;
    case CAPS_LOCK:
        caps_lock_pressed = !caps_lock_pressed;     // this is a toggle state per osdev, so just flip state
        break;
    case CAPS_LOCK_RELEASE:
        break;
    case ENTER:
        keyboard_return();
        break;
    case BACKSPACE:
        if(buf_idx > 0){
            delete_from_buf();
            backspace();    // if we can delete a char from screen, call backspace (lib.c) to erase
        }
        break;
    // otherwise normal
    default:
        process_key(scancode);
        break;
    }

    // at the end send an EOI
    send_eoi(KEYBOARD_IRQ_LINE);
    sti();
}

/*
 * process_key
 *   DESCRIPTION: Determines which ASCII letter to add to buffer
 *   INPUTS: scancode - key to process
 *          state - which key state is active
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: calls function to add to buffer
 */   
void process_key(uint8_t scancode){
    uint8_t idx1;   // define indexes
    uint8_t idx2;
    char letter;    // letter that we add to buffer

    // 0x2D X pressed, 0xAD X released -> diff of 0x80 -> and 0x80!
    if(scancode & 0x80){
        return;                         // release so just skip and send interrupt
    }

    idx1 = (scancode & 0xF0) >> 4;      // grab high 4 bits to determine group, rshift by 4 to get row
    idx2 = (scancode & 0x0F);           // grab low 4 bits to determine specific char

    if(idx1 > 3){
        return;     // OOB since does not exist in my mappings
    }

    // grab letter as normal - if no coniditons are met we keep normal mapping
    letter = normal_map[idx1][idx2];

    if(ctrl_pressed){
        // clear screen if CTRL+L
        if(scancode == L){
            clear();                    // clear screen
        }
        return;
    }
    if(alt_pressed){
        return;     // do not do anything (yet)
    }
    if(caps_lock_pressed){
        letter = shift_map[idx1][idx2];

        // anything not letters should not shift
        // upper case goes from 65 to 90, so check if not a letter (i.e. a number)
        if(!(65 <= letter && letter <= 90)){
            letter = normal_map[idx1][idx2];
        }
    }
    if(shift_pressed){
        // check if caps_lock_pressed -> if so reverse letter scheme
        if(caps_lock_pressed){
            // if letter was made to be capital -> upper case goes from 65 to 90
            if(65 <= letter && letter <= 90){
                letter = normal_map[idx1][idx2];
            }
            else{
                letter = shift_map[idx1][idx2];     // shift otherwise
            }
        }
        // caps_lock not pressed, proceed normally
        else{
            letter = shift_map[idx1][idx2];     // shift otherwise
        }
    }

    // // tell terminal driver that a key was pressed
    // flag = 1;
    
    // add letter to current buffer
    add_to_buf(letter);
    putc(letter);
}


/*
 * add_to_buffer
 *   DESCRIPTION: Adds determined ASCII letter to buffer
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: does not add anymore letters if buffer is full, increments buf_idx when adding
 */   
void add_to_buf(char letter){
    // we reserve index 127 (keyboard_buf_size-1) for new line
    if(buf_idx < KEYBOARD_BUF_SIZE-1){
        keyboard_buf[buf_idx] = letter;
        buf_idx++;
    }
}


/*
 * delete_from_buf
 *   DESCRIPTION: Deletes last character from buffer
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: decrements buf_idx when deleting, only deletes if char exists
 */ 
void delete_from_buf(void){
    // make current buf_idx null
    keyboard_buf[buf_idx] = '\0';

    // if not at start, decrement buf_idx
    if(buf_idx > 0){
        buf_idx--;
    }
}


/*
 * clear_keyboard_buf
 *   DESCRIPTION: Clears all characters from buffer (setting all chars to null)
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: resets buf_idx to 0
 */ 
void clear_keyboard_buf(void){
    // clear all characters to be null
    int i;
    for(i = 0; i < KEYBOARD_BUF_SIZE; i++){
        keyboard_buf[i] = '\0';
    }

    // reset index and flag to 0
    buf_idx = 0;
}

/*
 * keyboard_return
 *   DESCRIPTION: Returns current keyboard buffer to terminal read (maybe new line?)
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: clears keyboard buffer afterwards
 */ 
void keyboard_return(void){
    putc('\n');
    keyboard_buf[buf_idx] = '\n';       // insert into buffer

    buf_idx++;
    
    // tell terminal driver that a key was pressed
    flag = 1;
}
