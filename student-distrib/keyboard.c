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

    // have buf_idx start at beginning
    buf_idx = 0;
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
    // cli();      // begin crit section

    // press key -> generates interrupt -> calls handler (THIS)
    //  -> figure out what key was pressed and process it (print to screen)

    uint8_t scancode;
    scancode = inb(KEYBOARD_PORT);      // read scancode from keyboard port

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
    case TAB:
        autocomplete();
        break;
    case TAB_RELEASE:
        break;
    case BACKSPACE:
        if(buf_idx >= 0){
            if(buf_idx > 0) backspace();    // if we can delete a char from screen, call backspace (lib.c) to erase
            delete_from_buf();
        }
        break;
    // otherwise normal
    default:
        process_key(scancode);
        break;
    }

    // at the end send an EOI
    send_eoi(KEYBOARD_IRQ_LINE);
    // sti();

    // if CTRL+C and process (par 1st shell) is running, cancel that
    if(running_flag == 1 && call_halt == 1){
        // indicate the process no longer running
        running_flag = 0;
        call_halt = 0;
        sys_halt(-1);
    }
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
        return;     // OOB since does not exist in my mappings (0 <= idx1 <= 3)
    }

    // grab letter as normal - if no coniditons are met we keep normal mapping
    letter = normal_map[idx1][idx2];

    if(ctrl_pressed){
        // clear screen if CTRL+L or CTRL+l
        if(scancode == L){
            clear();                    // clear screen from lib.c
            
            // if shell is running, print 391OS>
            if(shell_flag >= 1){
                printf("391OS> ");
            }

            // reprint keyboard buffer
            // if(keyboard_buf[0] != '\0'){
            //     int i;
            //     for(i = 0; i < strlen(keyboard_buf); i++){
            //         putc(keyboard_buf[i]);
            //     }
            // }
            int i;
            for(i = 0; i < strlen(keyboard_buf); i++){
                putc(keyboard_buf[i]);
            }
        }

        // cancel current program
        if(scancode == C){
            // sys_halt(-1);       // -1 since we are terminating process before it can finish
            call_halt = 1;
        }
        return;
    }
    if(alt_pressed){
        // pending on F#, switch to terminal #
        switch (scancode)
        {
        case F1:
            switch_terminals(1);
            // curr_term = 1;
            break;
        case F2:
            switch_terminals(2);
            // curr_term = 2;
            break;
        case F3:
            switch_terminals(3);
            // curr_term = 3;
            break;
        default:
            break;
        }

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
    
    // // add letter to current buffer
    // add_to_buf(letter);
    // putc(letter);

    // only print when <=127 characters, so idx must be at most 126
    if(buf_idx <= 126){
        // add letter to current buffer
        add_to_buf(letter);
        putc(letter);
    }
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
    putc('\n');                         // print new line
    keyboard_buf[buf_idx] = '\n';       // insert into buffer

    buf_idx++;      // not necessary but kept for debugging -> gets reset anyway
    
    // tell terminal driver that '\n' was pressed
    key_flag = 1;
    // clear_keyboard_buf();       // need to clear the buffer at the end
}


/*
 * autcomplete
 *   DESCRIPTION: Implementation of bash autocomplete when pressing TAB
 *   SIDE EFFECTS: adds to buffer and prints to screen if valid match
 */ 
void autocomplete(void){

    dentry_t cur_dentry;
	int i, j, k, matches, length, old_length, old_idx;
    char prefix[33];
    char best[33];
    char check[33];
    char temp[33];

    for(i = 0; i < 33; i++){
        prefix[i] = '\0';
        best[i] = '\0';
        temp[i] = '\0';
    }

    // we will compare at most 30 elements;
    int8_t ls[30][33];

    // get prefix that is after LAST space
    length = strlen(keyboard_buf);
    old_length = length;
    matches = 0;

    for(i = length-1; i >= 0; i--){
        if(i >= 1 && keyboard_buf[i-1] == ' ') break;
    }

    old_idx = i;        // record index of space
    
    if(keyboard_buf[i+1] == ' ' || keyboard_buf[i+1] == '\0'){
        return;     // do nothing since space was last thing typed
    }
    else{
        // strncpy(prefix, keyboard_buf+i+1, length-i);
        strncpy(prefix, keyboard_buf+i, length-i+1);
    }

    // strncpy(prefix, keyboard_buf, KEYBOARD_BUF_SIZE);
    length = strlen(prefix);        // make new length

    i = 0;
    k = 0;
    // loop through all possible files in directory
	while ( read_dentry_by_index(i, &cur_dentry) != -1 ) {

		/* Create name buffer to copy fname into + one extra space for null '\0' char */
		uint8_t fname[FILENAME_LEN + 1];
		for(j = 0; j < FILENAME_LEN; j++){
			fname[j] = cur_dentry.filename[j];
		}

		fname[32] = '\0';	                    //terminate any string over 32 chars with null char
        // strncpy(ls[i], (int8_t*)fname, 33);     // 33 is size of fname

        // see if prefixes match
        memcpy(check, &fname, length);      // get substring/prefix of new file to look at

        if(strncmp((int8_t*)prefix, (int8_t*)check, length) == 0){
            strncpy((int8_t*)best, (int8_t*)fname, strlen((int8_t*)fname));
            matches++;
            strncpy(ls[k++], (int8_t*)fname, 33);     // 33 is size of fname, copy into ls when matching
        }

        i++;
	}

    if(matches == 0) return;        // nothing in directory, so just return and do nothing

    // check if there were duplicates -> get min value to compare with
    if(matches > 1){
        int index;
        int32_t min = 10000000;                     //equivalent to infinity, need to compare mins
        for(index = 1; index < i; index++){
            // int32_t temp_len = strncmp(ls[index], ls[index-1], strlen(ls[index-1]));
            // temp_len = (temp_len < 0) ? strlen(ls[index]) : strlen(ls[index-1]);
            // min = (temp_len < min) ? temp_len : min;

            if(ls[index-1][0] == '\0' || ls[index][0] == '\0') continue;        // do not compare if empty

            int rel_min = (strlen(ls[index-1]) < strlen(ls[index])) ? strlen(ls[index-1]) : strlen(ls[index]);
            for(j = 0; j <= rel_min; j++){
                if(ls[index-1][j+1] != ls[index][j+1]){
                    // identified index of difference -> need to compare with previous min
                    min = (j < min) ? j : min;
                }
            }
        }

        /* now we have the minimum index j
            cat fra
                frame1.txt
                frame0.txt
                frameframeframeframe.txt
            should go to:
            cat frame (j = 4)
        */
        if(length-1 < min){
            // change best
            strncpy(temp, best, 33);        // 33 is size of these names
            memcpy(best, temp, min+1);       // get substr of best (0 to min+1)
            best[min+1] = '\0';
        }
        else{
            return;     // duplicates exist, already at max prefix thats the same
        }
        

    }

    // replace keyboard buffer
    j = 0;
    for(i = old_idx; i < old_idx+1+strlen((int8_t*)best); i++){
        keyboard_buf[i] = best[j];
        j++;
    }

    // erase old buffer
    for(i = old_length-1; i > old_idx; i--){
        backspace();
    }

    // place new buffer
    // strncpy(keyboard_buf, best+old_idx, strlen(best));
    // NEED TO UPDATE BUF_IDX I THINK FUCCCCCCCKKKKK
    buf_idx = strlen((int8_t*)keyboard_buf);

    for(i = old_idx+1; i < KEYBOARD_BUF_SIZE; i++){
        putc(keyboard_buf[i]);
    }

    // printf("\nkeyboard_buf: %d\n", strlen((int8_t*)keyboard_buf));
    // printf("buf_idx: %d", buf_idx);

}

