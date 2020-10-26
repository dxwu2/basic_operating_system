//initialized rtc XD
//https://wiki.osdev.org/RTC
#include "i8259.h"
#include "lib.h"
#include "rtc.h"
/*
 * rtc_init
 *   DESCRIPTION: Initializes rtc for usage 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Enables IRQ on pic, outputs data to RTC data port
 */   
//frequency =  32768 >> (rate-1);
volatile int rtc_interrupt_occurred;
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
 * rtc_open
 *   DESCRIPTION: open rtc set frequency to default 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS:changes rtc frequency
 */ 
int32_t rtc_open(const uint8_t* filename) {
    set_rtcFrequency(2);
    return 0;
}
/*
 * rtc_close
 *   DESCRIPTION: close rtc
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: none
 */ 
int32_t rtc_close(int32_t fd) {
    return 0;
}
/*
 * rtc_read
 *   DESCRIPTION: read from rtc 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: waits until interrupt has occured
 */ 

int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes) {
    rtc_interrupt_occurred = 0;
    while(!rtc_interrupt_occurred){
    }
    return 0;
}
/*
 * rtc_write
 *   DESCRIPTION: writes a frequency to the rtc
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: -1 on failure 0 on success 
 *   SIDE EFFECTS: modifies rtc interrupt frequency
 */ 
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes) {
    if (set_rtcFrequency(*(int*)(buf)) == -1)
        return -1;
    return 0;
}

/*
 * set_rtcFrequency
 *   DESCRIPTION: changes the RTC frequency
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: -1 on failure 0 on success
 *   SIDE EFFECTS: changes the frequency of RTC
 */
int set_rtcFrequency (int frequency) {
    int rate;
    if (frequency < 2 || frequency > 1024)
        return -1;
    if (frequency == 2)
	    rate = 0x0F;
	if (frequency == 4)
		rate = 0x0E;
	if (frequency == 8)
		rate = 0x0D;
	if (frequency == 16)
		rate = 0x0c;
	if (frequency == 32)
		rate = 0x0b;
	if (frequency == 64)
		rate = 0x0a;
	if (frequency == 128)
		rate = 0x09;
	if (frequency == 256)
		rate = 0x08;
	if (frequency == 512)
		rate = 0x07;
	if (frequency == 1024)
		rate = 0x06;
    cli();
	outb(SREG_A, IDX_PORT);  //select regA
	char a_val = inb(DATA_PORT); //hold the value of reg A
	outb(SREG_A, IDX_PORT); //select regA again
	outb((a_val & 0xF0) | rate, DATA_PORT); //set rate
	sti();
    return 0;
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
    rtc_interrupt_occurred = 1;
    //test_interrupts();
    send_eoi(8); //send eoi to RTC
    sti(); //restore
    return;
}
