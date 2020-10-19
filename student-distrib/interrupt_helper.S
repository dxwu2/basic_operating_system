.text

.globl KEYBOARD_INTERRUPT

# KEYBOARD_INTERRUPT(void);
# Interrupt called when key is pressed. Pushes all the registers/flags and pops them to save state
# Inputs   : none
# Outputs  : none
KEYBOARD_INTERRUPT:
        cli         # begin critical section
        pushal      # pushing all registers
        pushfl      # pushing all flags

        call keyboard_handler

        popfl       # popping all flags
        popal       # popping all registers
        sti
        ret
