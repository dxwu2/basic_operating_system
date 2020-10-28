.globl  SYSTEM_CALL_WRAPPER

# SYSTEM_CALL(void);
# Assembly linkage for system call handler triggered by "int 0x80"
# Inputs   : none
# Outputs  : none
SYSTEM_CALL_WRAPPER:
    cli             # begin critical section
    pushl %edx      # push argument registers for system call
    pushl %ecx
    pushl %ebx
    pushfl          # pushing all flags
    cmpl $1, %eax   # check system call arg is between 1 and 8 (for 8 system calls total) 
    jb inval_arg
    cmpl $8 %eax
    ja inval_arg

    call *jump_table(, %eax, 4) # call corresponding system call from jump table
                                # (index stored in eax * 4 bytes per entry in table)
    jmp cleanup_call
    
inval_arg: 
    orl $0xFFFFFFFF, %EAX   

cleanup_call:
    popfl       # popping all flags
    popl %ebx   # popping all caller-saved registers
    popl %ecx
    popl %edx
    sti         # end critical section
    iret        # per osdev, need iret since interrupt context

jump_table:
    .long 0x0
    .long sys_halt
    .long sys_execute
    .long sys_read
    .long sys_write
    .long sys_open
    .long sys_close
    .long sys_getargs
    .long sys_vidmap