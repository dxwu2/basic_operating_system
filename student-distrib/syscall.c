
#include "syscall.h"
#include "filesystem.h"
#include "terminal.h"

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

    return 0;
}


int32_t sys_read (int32_t fd, void* buf, int32_t nbytes){
    return 0;
}


int32_t sys_write (int32_t fd, void* buf, int32_t nbytes){
    return 0;
}


int32_t sys_open (const uint8_t* filename){
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
