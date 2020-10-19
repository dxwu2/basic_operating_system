//initialized rtc XD
//https://wiki.osdev.org/RTC
#include "rtc.h"
#include "i8259.h"
#include "lib.h"

/*
 * rtc_init
 *   DESCRIPTION: Initializes rtc for usage 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Enables IRQ on pic, outputs data to RTC data port
 */   
//frequency =  32768 >> (rate-1);
void rtc_init(){
    //disable interrupts
    // cli(); 
    outb(SREG_B, IDX_PORT); //select regB
    char b_val = inb(DATA_PORT); //hold the value of regB
    outb(SREG_B, IDX_PORT); //select regB again
    outb(b_val | 0x40, DATA_PORT); //set the 6th bit of reg B to 1 to turn on periodic interrupts 
    int rate = 0x0F; //initialized at 2hz
    outb(SREG_A, IDX_PORT);  //select regA
	char a_val = inb(DATA_PORT); //hold the value of reg A
	outb(SREG_A, IDX_PORT); //select regA again
	outb((a_val & 0xF0) | rate, DATA_PORT); //set rate
    outb(SREG_C, IDX_PORT);	// select register C
    inb(DATA_PORT);		//throw away contents
    enable_irq(8);  //enable irq
    return;
}


/*
 * rtc_handler
 *   DESCRIPTION: Handler code to be invoked when an RTC interrupt is called 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Enables IRQ on pic, outputs data to RTC data port
 */   
void rtc_handler(){
    //disable interrupts
    cli(); 
    outb(SREG_C, IDX_PORT);	// select register C
    inb(DATA_PORT);		//throw away contents
    test_interrupts();
    send_eoi(8); //send eoi to RTC
    sti(); //restore
    return;
}
