
#include "schedule.h"
#include "lib.h"


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

    // output freq * counter

    sti();
}

