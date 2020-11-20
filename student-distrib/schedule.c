
#include "schedule.h"
#include "lib.h"
#include "paging.h"
#include "syscall.h"

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
    int curr_pid, next_pid;
    if(something){
        switch_process(curr_pid, next_pid);
    }
}

/*Change currently scheduled process to next in scheduling queue*/
void switch_process(int curr_pid, int next_pid){
    /*Remap user 128MB to new user program*/
    map_user_program(next_pid);

    /*Fetch pcb for current and next process*/
    pcb_t* next_pcb = get_pcb_from_pid(next_pid);
    // pcb_t* curr_pcb = get_pcb_from_pid(curr_pid);

    if (next_pcb->term_id == curr_term){
        /*Remap video memory to visible region if we are executing on current terminal??*/
        map_vidmem();
    }

    /*Switch ESP and EBP to next processes kernel stack*/
     asm volatile(
        // literally save ebp and esp into (free to clobber) registers
        "movl %0, %%ebp;"
        "movl %1, %%esp;"
        
        : // no outputs
        : "r"(next_pcb->old_ebp), "r"(next_pcb->old_esp)    //we might need way to save esp/ebp after context switch in execute (instead of old parent's esp/ebp)
    );

    /*Restore next process' TSS*/
    tss.ss0 = KERNEL_DS;
    tss.esp0 = next_pcb->base_kernel_stack;
}