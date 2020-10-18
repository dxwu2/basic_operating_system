//RTC header + definitions
#define	IDX_PORT	    0x70
#define DATA_PORT	    0x71
#define SREG_A		    0x8A
#define SREG_B		    0x8B
#define SREG_C		    0x8C
#define IRQ_PORT        0x08

//used to initialize the RTC
void rtc_init();
//simple handler for rtc checkpoint one test
void rtc_handler();
