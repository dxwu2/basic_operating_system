.globl  SYSTEM_CALL_WRAPPER

# SYSTEM_CALL(void);
# Assembly linkage for system call handler triggered by "int 0x80"
# Inputs   : none
# Outputs  : none
SYSTEM_CALL_WRAPPER:
        cli         # begin critical section
        pushal      # pushing all registers
        pushfl      # pushing all flags
        

        call system_call

        popfl       # popping all flags
        popal       # popping all registers
        sti         # end critical section
        iret        # per osdev, need iret since interrupt context

jump_table:
# .long syscall1, syscall2