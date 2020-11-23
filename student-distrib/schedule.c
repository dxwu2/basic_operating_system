
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

/* initial_boot()
 * Setups terminals and performs initial boot method
 * no input, no output
 * no side effects
 */
void initial_boot() {
    int i, j;
    // inititalize 3 terminals
    for (i = 0; i < 3; i++){
        term_t term;
        term.term_id = i;
        term.term_x = 0;
        term.term_y = 0;
        for(j = 0; j < KEYBOARD_BUF_SIZE; j++){
            term.keyboard_buf[j] = '\0';
        }
        /*Map each of terminals to corresponding vidmem page*/
        vidmap_term(i);
        
        terminals[i] = term;
    }

    terminals[0].vidmem = (int32_t*) TERM_1_VIDPAGE;
    terminals[1].vidmem = (int32_t*) TERM_2_VIDPAGE;
    terminals[2].vidmem = (int32_t*) TERM_3_VIDPAGE;

    //initialize 3 shell processes
    for (i = 0; i < 3; i++) {
        curr_term = i;
        sys_execute((uint8_t*)"shell");

        // need to map for shell2 and shell3
        if(i >= 1) {
            scheduling_vidmap(i);
            // map_user_program(i);
        }
    }

    //Reset curr_term to first shell terminal
    curr_term = 0;

    // set process to be 2 in preparation for the first terminal, and then start round robin
    scheduled_process = 2;
}

/*Change currently scheduled process to next in scheduling queue*/
void schedule(){

    int next_scheduling_term;    

    // --------------------------------------------- normal scheduling --------------------------------------------- //

    // get current pcb and next pcb
    pcb_t* curr_pcb;
    pcb_t* next_pcb;
    curr_pcb = get_pcb_from_pid(scheduling_array[scheduled_process]);
    next_scheduling_term = (++scheduled_process % 3);
    next_pcb = get_pcb_from_pid(scheduling_array[next_scheduling_term]);

    // when do we actually edit the pid# in scheduling_array

    // video remapping
    // if scheduled_process == curr_term, then don't need to do anything because it's already mapped to physical vid addr (0xB8000), we did this in checkpoint 4
    // if NOT equal, then need to change mapping to map to background buffers in physical memory
    if(scheduled_process != curr_term){
        // something
        scheduling_vidmap(next_scheduling_term);        // represents the NEXT terminal (w/in [0,2])
    }

    /*Remap user 128MB to new user program*/
    map_user_program(scheduling_array[next_scheduling_term]);

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
