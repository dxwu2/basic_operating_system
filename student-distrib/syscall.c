
#include "syscall.h"
#include "filesystem.h"
#include "terminal.h"
#include "rtc.h"

// initialize the file operations table 
fops_t std_in_table = {bad_call, terminal_read, bad_call, bad_call};
fops_t std_out_table = {bad_call, bad_call, terminal_write, bad_call};
fops_t rtc_table = {rtc_open, rtc_read, rtc_write, rtc_close};
fops_t filesys_table = {file_open, file_read, file_write, file_close};
fops_t filedir_table = {dir_open, dir_read, dir_write, dir_close};
fops_t bad_table = {bad_call, bad_call, bad_call, bad_call};

/* local variables */
static int pid_array[MAX_NUM_PIDS];


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


// insert function interface
int32_t sys_halt (uint8_t status){
    // call sti since we called cli in execute

    // CHECK IF HALTING SHELL (hints doc thingy)
    // if it is the first shell, just return -1 => literally use like an if statement

    pcb_t* curr_pcb = get_pcb_ptr();

    // Set pids to unused
    int i;
    // we start at 2 because of stdin and stdout
    for (i = 2; i < FDA_SIZE; i++) {
        curr_pcb->fda[i].flags = NOT_IN_USE;
    }

    // restore parent paging
    // pcb_t* parent_pcb = curr_pcb->parent_pcb;
    map_user_program(curr_pcb->parent_pid);

    // close any relevant FDs
    // STILL NEED TO DO?
    
    // restore tss values and EBP/ESP values
    tss.esp0 = curr_pcb->old_esp;

    asm volatile(
        // literally save ebp and esp into (free to clobber) registers
        "movl %0, %%esp;"
        "movl %1, %%ebp;"
        "movl %2, %%eax;"
        "jmp RETURN_FROM_HALT;"
        
        : // no outputs
        : "r"(curr_pcb->old_esp), "r"(curr_pcb->old_ebp), "r"((uint32_t)status)       // we booling now
        : "eax"
    );

    // remember to go back to end of execute and set the return value accordingly (always returns 0 right now)
    return 0;
}
    

/* int32_t sys_execute (const uint8_t command)
 * Attempts to load and execute a new program -> handing off the processor to the new program until it terminates
 * Inputs: uint8_t command - reads command to know what to do
 * Outputs: -1 if command cannot be executed
 *          256 if program dies by an exception
 *          [0,255] if program executes a halt syscall (value returned given by call to halt)
 * Side Effects: Does a lot of stuff
 */
int32_t sys_execute (const uint8_t* command){

    // STEP 1: parse the command
    int i;

    if(command == NULL){
        return -1;
    }

    int8_t cmd[MAX_CMD_LENGTH];                 // get first command
    uint8_t args[MAX_ARGS_LENGTH];              // get following arguments
    uint8_t idx = 0;                            // to identify spaces (also to index cmd)
    uint8_t args_idx = 0;                       // to index args

    for(i = 0; i < MAX_CMD_LENGTH; i++){
        cmd[i] = '\0';
        args[i] = '\0';
    }

    while(command[idx] != ' ' && command[idx] != '\0'){
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
        return -1;                              // return -1 since dentry does not exist
    }

    // read header (found at start of ELF file)
    uint8_t read_buf[4];                               // only need 4 because of the magic numbers
    uint32_t inode_idx = d.inode_num;                   // get inode from read_dentry
    read_data(inode_idx, 0, read_buf, 4);               // again, only 4 magic numbers, read into read_buf

    // now check if executable file
    // magic numbers from https://wiki.osdev.org/ELF (0x7F, E, L, F) -> bits 0 to 3
    if(read_buf[0] != 0x7F || read_buf[1] != 'E' || read_buf[2] != 'L' || read_buf[3] != 'F'){
        return -1;      // return -1 since magic numbers dont match what ELF should be
    }

    // now obtain program entry position (bits 24-27) - also 4 bits so we can overwrite read_buf
    // offset should be 6 since we need to start looking at 24 bytes (we originally start from 0)
    // the offset is a uint32_t, so there are 4 bytes per (24/4 => 6)
    read_data(inode_idx, 6, read_buf, 4);              // read 4 bc only 4 bits to read
    uint32_t entry_position = (uint32_t)(read_buf[3] << 24 | read_buf[2] << 16 | read_buf[1] << 8 | read_buf[0]); // shell gives 0x080495c0 (still wrong)
    
    //uint32_t entry_position = (uint32_t)read_buf;     // make this a uint32_t pointer -> shell gives 0x7ffe24 (def wrong)
    // uint32_t entry_position = *((uint32_t*)read_buf); // shell gives 0x080495c0
    //uint32_t entry_position = 0x080482e8;     // -> correct value according to hex editor


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
    pcb_t curr_pcb;
    init_pcb(&curr_pcb, pid, args);

    // if not shell, we must set a parent
    if(pid != 0){
        pcb_t* parent_pcb = get_pcb_ptr();          // getting existing pcb
        curr_pcb.parent_pid = parent_pcb->curr_pid;          // set this to be parent of (soon-to-be) new pcb
        parent_pcb->child_pid = curr_pcb.curr_pid;           // set parent's child to be curr_pcb
    }

    // save current EBP and ESP registers into PCB before we change
    asm volatile(
        // literally save ebp and esp into (free to clobber) registers
        "mov %%esp, %0;"
        "mov %%ebp, %1;"
        
        : "=r" (curr_pcb.old_esp), "=r" (curr_pcb.old_ebp)        // bro is this how we read registers im losing my minddddddd
    );

    // STEP 6: Context Switch - home stretch ?
    // fucking big brain moves at 2:57am

    // prepare for context switch: write new process' info to TSS
    tss.ss0 = KERNEL_DS;                        // if OSDEV tells you to jump off a cliff, would you do it? of course yes OSDEV legit
    tss.esp0 = curr_pcb.base_kernel_stack;     // already calculated, fucking genius

    // need to do a tss_flush, then enter ring 3 ? -> no tss flush, since only one tss for all processes (for this mp)

    /* OSDEV Order
    user data segment
    push current esp
    pushf (?)
    user code segment
    */

    asm volatile(
        // "cli;"
        // look at x86_desc.h macros for the values below. also just follow osdev [order]
        "pushl $0x002B;"            // push user data segement (SS)
        //"pushl %%esp;"              // push ESP -> wrong because we are pushing the kernel stack address. ESP here is 0x7ffd04 -> definitely wrong
                                    // not what we want because we are switching to user stack
                                    // We want the user's stack pointer (virt. mem.) 
                                    // should the user's stack pointer less than 132 MB (132 MB - 4 B) -> so that it doesn't go outside the page
        //"mov $0x002B, %%ax;"  -> don't need this?? (according to Harsh)
        //"mov %%ax, %%ds;"
        "movl $0x83FFFFC, %%eax;"          // 132 MB - 4 B => 0x8400000 - 0x4 => 0x83FFFFC
        "pushl %%eax;"

        //"pushl $0x8400000;"         // push ESP (132MB)
        "pushfl;"                   // push flags (EFLAGS)
        "pushl $0x0023;"            // push user code segment (CS)
        "movl %0, %%eax;"
        "pushl %%eax;"              // push the entry position (EIP) -> must be somewhere around 128 MB
        "iret;"                     // iret has to happen
        
        "RETURN_FROM_HALT:;"         // this is where we jump from halt (per discussion)
        "ret;"                      // something has to happen after we call halt, change to IRET maybe??
        // maybe jump to cleanup call??? (in syscall linkage)
        
        : // no outputs
        : "r" (entry_position)
        : "eax", "ecx"
    );


    /* this is THE approach */
    /* set up correct values for user-level EIP, CS, EFLAGS, ESP, and SS registers on the kernel-mode stack */
    /* then execute iret (allows for kernel -> user) instruction */

    // it will pop it off and then go to what it was pointing to (context-wise)
    // ls stuff

    // comes into assembly linkage
    // calls execute ls
    // 5 things
    // shell

    // technically should never return 
    return 0;
}


int32_t sys_read (int32_t fd, void* buf, int32_t nbytes){
    //I need access to curr_pcb
    pcb_t* curr_pcb;
    /* Check bounds of fd idx */
    if (fd < 0 || fd >= FDA_SIZE)
        return -1;
    
    // get pcb_ptr
    curr_pcb = get_pcb_ptr();

    /* Check valid buf input */
    if (buf == NULL)
        return -1;
    /* Check if fd index corresponds to valid fd */
    if (curr_pcb->fda[fd].flags == NOT_IN_USE)
        return -1;

    curr_pcb->fda[fd].fops_ptr.read(fd, buf, nbytes);
    return 0;
}


int32_t sys_write (int32_t fd, void* buf, int32_t nbytes){
    //I need access to curr_pcb
    pcb_t* curr_pcb;
    /* Check bounds of fd idx */
    if (fd < 0 || fd >= FDA_SIZE)
        return -1;

    // get pcb_ptr
    curr_pcb = get_pcb_ptr(); // this might be wrong

    /* Check valid buf input */
    if (buf == NULL)
        return -1;
    /* Check if fd index corresponds to valid fd */
    if (curr_pcb->fda[fd].flags == NOT_IN_USE)
        return -1;

    curr_pcb->fda[fd].fops_ptr.write(fd, buf, nbytes);
    return 0;
}


int32_t sys_open (const uint8_t* filename){
    // need to return -1 if (File Descriptor) array is full - see appendix A 8.2 last sentence 
    //I need access to curr_pcb
    pcb_t* curr_pcb;
    dentry_t test_dentry;
    /*Check if named file exists*/
    // if(read_dentry_by_name(filename, &test_dentry) == -1)  -- original, i casted (int8_t*); this something we should ask about
    if(read_dentry_by_name((int8_t*)filename, &test_dentry) == -1)
        return -1;

    // get pcb_ptr
    curr_pcb = get_pcb_ptr();
    /*Check if any fd entries are free*/
    int32_t fd_idx;
    for(fd_idx = 2; fd_idx < FDA_SIZE; fd_idx++){
        /* Check for empty fda entry */
        if(curr_pcb->fda[fd_idx].flags == NOT_IN_USE)
            break;
        if (fd_idx == FDA_SIZE - 1)
            return -1;   
    }
    /* Declare and initialize file descriptor to be placed in fda */
    files_t fd_entry;
    switch (test_dentry.filetype){
        case 0:     //RTC filetype
            fd_entry.fops_ptr = rtc_table;
            break;
        case 1:     //Directory filetype
            fd_entry.fops_ptr = filedir_table;
            break;
        case 2:     //Traditional file filetype
            fd_entry.fops_ptr = filesys_table;
            break;
    }
    /* Set up remaining fields for our fda entry */
    fd_entry.inode = test_dentry.inode_num;
    fd_entry.file_position = 0;
    fd_entry.flags = IN_USE;

    /* Finally set entry in fda for curr_pcb */
    curr_pcb->fda[fd_idx] = fd_entry;
    return fd_idx;
}


int32_t sys_close (int32_t fd){
    pcb_t* curr_pcb;

    /* Check bounds of fd idx (must be between 2 and 7)*/
    if (fd < 2 || fd >= FDA_SIZE)
        return -1;

    // get pcb_ptr
    curr_pcb = get_pcb_ptr();

    /* Check if fd already invalid */
    if (curr_pcb->fda[fd].flags == NOT_IN_USE)
        return -1;
    /* Make sure file closes successfully */
    if(curr_pcb->fda[fd].fops_ptr.close(fd) != 0)
        return -1;
    /* Finally set fd entry to not in use */
    curr_pcb->fda[fd].flags = NOT_IN_USE;
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
        curr_pcb->fda[i].fops_ptr = bad_table;       // FIXED: CAUSES PAGE FAULT EXCEPTION: because of how we initialized curr_pcb
        // curr_pcb->fda[i].fops_ptr.open = NOT_IN_USE;
        // curr_pcb->fda[i].fops_ptr.read = NOT_IN_USE;
        // curr_pcb->fda[i].fops_ptr.write = NOT_IN_USE;
        // curr_pcb->fda[i].fops_ptr.read = NOT_IN_USE;
        
        curr_pcb->fda[i].inode = -1;
        curr_pcb->fda[i].flags = NOT_IN_USE;
        curr_pcb->parent_pid = NULL;
        curr_pcb->child_pid = NULL;
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
    curr_pcb->base_kernel_stack = 0x800000 - (pid) * 0x2000;      // 8MB - (pid)*8kB
    curr_pcb->curr_pid = pid;
}

/* pcb_t* get_pcb_ptr()
 * Gets a pointer to the current pcb based on esp pointer
 * Inputs:  none
 * Outputs: pcb_t pointer to current pcb
 * Side Effects: none
 */
pcb_t* get_pcb_ptr(void){
    pcb_t* curr_ptr;

    asm volatile(
        "movl %0, %%eax;"
        "andl %%esp, %%eax;"      // load 0x2B into task state register
        "movl %%eax, %1;"
        : "=r" (curr_ptr)
        : "r" (PCB_MASK)
    );

    return curr_ptr;
}
