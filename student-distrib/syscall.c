
#include "syscall.h"
#include "filesystem.h"
#include "terminal.h"
#include "rtc.h"
#include "x86_desc.h"

// initialize the file operations table 
fops_t std_in_table = {bad_call, terminal_read, bad_call, bad_call};
fops_t std_out_table = {bad_call, bad_call, terminal_write, bad_call};
fops_t rtc_table = {rtc_open, rtc_read, rtc_write, rtc_close};
fops_t filesys_table = {file_open, file_read, file_write, file_close};
fops_t bad_table = {bad_call, bad_call, bad_call, bad_call};


/* void handle_system_call()
 * Temporary system call handler 
 * Inputs: None
 * Outputs: None
 * Side Effects: Prints "System call was called."
 */
void handle_system_call(){
    clear();
    printf("System call (vector 0x80) was called.\n");
    while(1);
}

/* set up structures and local variables used in passing in parameters for system calls */
void syscall_init() {
    int i;
    for (i = 0; i < MAX_NUM_PIDS; i++) {
        pid_array[i] = UNUSED;
    }
}

int32_t sys_halt (uint8_t status){
    // REMEMBER TO SET PID TO UNUSED
    return 0;
}
    

/* int32_t sys_execute (const uint8_t command)
 * Attempts to load and execute a new program -> handing off the processor to the new program until it terminates
 * Inputs: uint8_t command - reads command to know what to do
 * Outputs: -1 if command cannot be executed
 *          256 if program dies by an exception
 *          [0,255] if porgam executes a halt syscall (value returned given by call to halt)
 * Side Effects: Does a lot of stuff
 */
int32_t sys_execute (const uint8_t* command){

    // STEP 1: parse the command

    if(command == NULL){
        return -1;
    }

    uint8_t cmd[MAX_CMD_LENGTH];            // get first command
    uint8_t args[MAX_ARGS_LENGTH];          // get following arguments
    uint8_t idx;                            // to identify spaces (also to index cmd)
    uint8_t args_idx;                       // to index args

    while(command[idx] != ' '){
        cmd[idx] = command[idx];        // copy command into cmd until first space
        idx++;
    }

    while(command[idx] == ' '){
        idx++;                          // skip spaces until we reach argument
    }

    while(command[idx] != '\0'){
        args[args_idx++] = command[idx++];      // copy rest of command (nonspace) into args
    }


    // STEP 2: check file validity

    // each process points to a dentry object (which points to inode object)
    dentry_t d;
	if(read_dentry_by_name(cmd, &d) == -1){
        return -1;      // return -1 since dentry does not exist
    }

    // read header (found at start of ELF file)
    uint8_t* read_buf[4];                               // only need 4 because of the magic numbers
    uint32_t inode_idx = d.inode_num;                   // get inode from read_dentry
    read_data(inode_idx, 0, read_buf, 4);               // again, only 4 magic numbers, read into read_buf

    // now check if executable file
    // magic numbers from https://wiki.osdev.org/ELF (0x7F, E, L, F) -> bits 0 to 3
    if(read_buf[0] != 0x7F || read_buf[1] != 'E' || read_buf[2] != 'L' || read_buf[3] != 'F'){
        return -1;      // return -1 since magic numbers dont match what ELF should be
    }

    // now obtain program entry position (bits 24-27) - also 4 bits so we can overwrite read_buf
    // offset should be 24 since we need to start looking at 24 (we originally start from 0)
    read_data(inode_idx, 24, read_buf, 4);              // read 4 bc only 4 bits to read
    uint32_t* entry_position = (uint32_t*)read_buf;     // make this a uint32_t pointer?


    // STEP 3: Paging

    // find an unused pid (0-5)
    int pid;
    for (pid = 0; pid < MAX_NUM_PIDS; pid++) {
        if (pid_array[pid] == UNUSED) {
            pid_array[pid] = USED;
            break;
        }
        // if pid_array is full
        if (pid == MAX_NUM_PIDS - 1) {
            return -1;
        }
    }
    // map user program
    map_user_program(pid);

    // STEP 4: user level program loader
    uint8_t* usr_buf = (uint8_t*)0x8048000;                 // memory 
    uint32_t exe_length = ((inode_t*)(fs_start_addr + (inode_idx + 1) * BLOCK_SIZE))->length;       // get length
    read_data(inode_idx, 0, usr_buf, exe_length);       // load data into usr_buf
                                                        // offset of 0 since we want to read in from byte 0 (start of executable)
    // already have entry position from step 2

    // STEP 5: PCB

    // helper function to make/initialize a PCB
    pcb_t* curr_pcb;
    init_pcb(curr_pcb, pid, args);

    // save current EBP and ESP registers into PCB before we change
    asm volatile(
        // literally save ebp and esp into (free to clobber) registers
        "movl %%esp, %%eax;"
        "movl %%ebp, %%ecx;"
        
        : // no inputs
        : "r" (curr_pcb->old_esp), "r" (curr_pcb->old_ebp)        // bro is this how we read registers im losing my minddddddd
        : // fine to clobber
    );

    // STEP 6: Context Switch - home stretch ?
    
    // ay so i think I cracked it - we just dont do this checkpoint
    // nah fuck that we read manuals and shit
    /*  steps to glory (per OSDEV - https://wiki.osdev.org/Task_State_Segment)
            SS0 = kernel datasegment 
            ESP0 (kernel esp) = stack-pointer 
    */

    // fucking big brain moves at 2:57am

    // prepare for context switch
    tss.ss0 = KERNEL_DS;                        // if OSDEV tells you to jump off a cliff, would you do it? of course yes OSDEV legit
    tss.esp0 = curr_pcb->base_kernel_stack;     // already calculated, fucking genius

    // push IRET context to stack
    // now its asm volatile time, this is where boys become men
    // OSDEV to save us? https://wiki.osdev.org/Getting_to_Ring_3

    // need to do a tss_flush, then enter ring 3 ?
    // ***yo this is wrong ignore - we will need to do pair programming for this (but fortunately this is the last step)

    asm volatile(
        "mov $0x2B, %%ax;"      // load 0x2B into task state register
        "mov %%ax, %%;"

        "mov $0x23, %%ax;"
        "mov %%ax, %%ds;"
        "mov %%ax, %%es;"
        "mov %%ax, %%fs;"
        "mov %%ax, %%gs;"

        "movl "
        "iret;"     // iret has to happen somewhere bro i do not know
        
        : // no inputs
        : // there is an output but halt handles this
        : // fine to clobber
    );

    return 0;
}


int32_t sys_read (int32_t fd, void* buf, int32_t nbytes){
    return 0;
}


int32_t sys_write (int32_t fd, void* buf, int32_t nbytes){
    return 0;
}


int32_t sys_open (const uint8_t* filename){
    // need to return -1 if (File Descriptor) array is full - see appendix A 8.2 last sentence 
    // (let us know if ur confused how to access fda)
    return 0;
}


int32_t sys_close (int32_t fd){
    return 0;
}

/* int32_t sys_getargs (uint8_t* buf, int32_t nbytes)
 * Reads the program's command line arguments into a user-level buffer
 * Inputs: uint8_t* buf - user buffer to read arguments into
 *         int32_t nbytes - number of bytes to read
 * Outputs: -1 if no arguments or if arguments and a terminal null do not fit in the buffer
 *          0 on success
 * Side Effects: Does a lot of stuff
 */
int32_t sys_getargs (uint8_t* buf, int32_t nbytes){
    // we will prob need to interact with PCB for this
    return 0;
}


int32_t sys_vidmap (uint8_t** screen_start){
    return 0;
}


/* int32_t bad_call(void)
 * Is a bad call function because function pointer does not exist
 * Inputs: none
 * Outputs: -1 by default since bad call
 * Side Effects: none
 */
int32_t bad_call(void){
    return -1;
}

/* void init_pcb(void)
 * Initializes a given pcb for use
 * Inputs:  pcb_t curr_pcb - pcb to initialize
 *          int pid - identify the process
 *          args - keep track of the arguments saved
 * Outputs: none
 * Side Effects: ?
 */
void init_pcb(pcb_t* curr_pcb, int pid, uint8_t* args){
    // initialize all entires to be bad at first
    int i;
    for(i = 0; i < FDA_SIZE; i++){
        curr_pcb->fda[i].fops_ptr = bad_table;
        curr_pcb->fda[i].inode = -1;
        curr_pcb->fda[i].flags = NOT_IN_USE;
        curr_pcb->parent_pcb = NULL;
        curr_pcb->child_pcb = NULL;
    }

    // initialize first two entries of fda to STDIN and STDOUT
    curr_pcb->fda[0].fops_ptr = std_in_table;
    // curr_pcb->fda[0].tss.esp0inode = -1;                 // stdin should not have inode
    curr_pcb->fda[0].flags = IN_USE;
    curr_pcb->fda[1].fops_ptr = std_out_table;
    // curr_pcb->fda[1].inode = -1;                 // stdout should not have inode
    curr_pcb->fda[1].flags = IN_USE;

    // curr_pcb->old_esp = ;
    // curr_pcb->old_ebp = tss.eb

    // set base kernel stack (depends on the pid)
    curr_pcb->base_kernel_stack = 0x800000 - (pid+1) * 0x2000;      // 8MB - (pid+1)*8kB
}

