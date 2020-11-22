
#include "schedule.h"

// initializes the PIT
void init_PIT(){
    //use channel 0 data port (0x40)
    //use Mode/Command register port (0x43) to write instructions
    cli();
    outb(SET_MODE, COMMAND_REG);    //set PIT to generate square waves in 16-bit binary on channel 0 with lo/hi access mode

    // we want 10 to 50 ms
    outb(COUNTER_LO, PIT_DATA);     //send low 8-bits of counter followed by high 8 bits (counter = 59659 because 1193180/counter = 20Hz (50ms))
    outb(COUNTER_HI, PIT_DATA);

    enable_irq(PIT_IRQ);      //Enable IRQ on port 0 (timer chip)
    sti();

    // initialize scheduling array
    int i;
    for(i = 0; i < 3; i++){
        scheduling_array[i] = -1;       // initialize to -1 to indicate not booted
    }
}

/*Function called upon interrupt fired by PIT*/
void PIT_handler(){
    send_eoi(PIT_IRQ);
    /*If more than one active terminal running, switch process*/
    schedule();

    /*
    need a way to keep track of list of pids
    current pid is important, as well as order
    i.e. fish1 (T1) -> fish3 (T3) -> fish2 (T2)
    
    how to account for when program is done? do round robin
    */

}

/*Change currently scheduled process to next in scheduling queue*/
void schedule(){

    int next_scheduling_term;

    // check if terminal 1 booted
    if(scheduling_array[0] == -1){
        scheduling_array[0] = 0;        // 0th index of pid array
        sys_execute((uint8_t*)"shell");
        return;
    }

    // TODO: initial boot up method

    // // check if currently scheduled process is booted or not
    // if(scheduling_array[curr_executing_term] == -1){
    //     curr_executing_term++;
    //     if (curr_executing_term == 3) {
    //         curr_executing_term = 0;
    //     }
    //     return;
    // }

    // --------------------------------------------- normal scheduling --------------------------------------------- //

    // get current pcb and next pcb
    pcb_t* curr_pcb;
    pcb_t* next_pcb;
    curr_pcb = get_pcb_from_pid(scheduling_array[scheduled_process]);
    next_scheduling_term = (++scheduled_process % 3);
    next_pcb = get_pcb_from_pid(scheduling_array[next_scheduling_term]);

    // when do we actually edit the pid# in scheduling_array

    // video remapping
    // if scheduled_process == viewing_process, then don't need to do anything because it's already mapped to physical vid addr (0xB8000), we did this in checkpoint 4
    // if NOT equal, then need to change mapping to map to background buffers in physical memory
    if(scheduled_process != viewing_process){
        // something
        scheduling_vidmap(next_scheduling_term);        // represents the NEXT terminal (w/in [0,2])
    }

    /*Remap user 128MB to new user program*/
    map_user_program(scheduling_array[next_scheduling_term]);

    // pcb_t* curr_pcb = get_pcb_from_pid(curr_pid);

    // if (next_pcb->term_id == curr_term){
    //     /*Remap video memory to visible region if we are executing on current terminal??*/
    //     map_vidmem();
    //     // vidmap_term();
    // }

    /* Switch ESP and EBP to next processes kernel stack */
     asm volatile(
        // literally save ebp and esp into (free to clobber) registers
        "movl %0, %%ebp;"
        "movl %1, %%esp;"
        
        : // no outputs
        : "r"(next_pcb->old_ebp), "r"(next_pcb->old_esp)    //we might need way to save esp/ebp after context switch in execute (instead of old parent's esp/ebp)
    );

    /* Restore next process' TSS */
    tss.ss0 = KERNEL_DS;
    // tss.esp0 = next_pcb->base_kernel_stack;
    tss.esp0 = 0x800000 - (scheduling_array[next_scheduling_term]) * 0x2000;      // 8MB - (pid)*8kB

    // TODO: update the curr executing term and/or the pid in the scheduling array
    scheduled_process = next_scheduling_term;
}

/*

kernel:
4MB

4
3
2
1
0
8MB


p3
p2
p1
shell3
shell2
shell1
[base]


pid array[6]
once we find the process#, we can get pcb

s[term1] == false

s[term2] == false

s[term3] == false

*/

