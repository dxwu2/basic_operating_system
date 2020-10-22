#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "terminal.h"

#define PASS 1
#define FAIL 0
#define BUFSIZE 128		// for testing terminal_read/write

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 15; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}
	for (i = 16; i < 20; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/* Divide-by-zero Test
*
* Asserts that division by zero prints correct exception message
* Inputs: None
* Outputs: None
* Side Effects: Prints exception message
* Coverage: idt_setup(), handle_exception()
* Files: exceptions.c/h, idt_setup.c/h
*/
void divide_by_zero_test(){
	TEST_HEADER;

	int result;
	int op = 1;
	int div = 0;
	result = op/div;
}

/* System call Test
*
* Asserts system call goes through
* Inputs: None
* Outputs: None
* Side Effects: Prints "exception message"
* Coverage: idt_setup(), handle_system_call()
* Files: exceptions.c/h, idt_setup.c/h
*/

void system_call_test(){
	TEST_HEADER;

	asm volatile ("int $0x80");
}

/* Paging tests 1 & 2- tests that accessible locations are accessible for vid mem
 * 
 * Inputs	: None
 * Outputs	: None
 * Side Effects	: Prints exception message
 */
 int paging_test1() {
	 TEST_HEADER;

	 int result = PASS;

	 //int a = 0xB8000, b;
	 int a = 0x800000 + 8;
	 int b;
	 int* ptr;	// uninitialized pointer
	 ptr = &a;	// store the address of a in ptr
	 b = *ptr;	// place value at ptr in b

	 return result;
 }

/* Tests if Page Fault Exception fired on dereferencing null ptr
 * 
 * Inputs	: None
 * Outputs	: None
 * Side Effects	: Prints Page Fault exception message
 */
void null_test(){
	TEST_HEADER;

	int* ptr = NULL;
	int test;
	test = *ptr;
}

/* Checkpoint 2 tests */

/* Tests if terminal_read/write properly reads data from keyboard buffer
	and writes data to terminal correctly
 * 
 * Inputs	: None
 * Outputs	: None
 * Side Effects	: Echos whatever was typed
 */
void terminal_test1(){
	int32_t cnt;
	char* buf;
	char* echo;
	char* terminal;

	while(1){
		terminal = "391OS> ";
		echo = "You said: ";
		terminal_write(1, terminal, 7);

		while(keyboard_buf[strlen(keyboard_buf)-1] != '\n');

		if(keyboard_buf[0] == '\n'){
			clear_keyboard_buf();
			continue;
		}
		cnt = terminal_read(1, buf, KEYBOARD_BUF_SIZE);
		
		terminal_write(1, echo, 10);
		terminal_write(1, buf, cnt);
	}
}

/* Tests if terminal_read/write properly reads data from keyboard buffer
	and writes data to terminal correctly
 * 
 * Inputs	: None
 * Outputs	: None
 * Side Effects	: While loop of terminal_read and terminal_write per doc
 */
void terminal_test2(){
	int32_t cnt;
	char* buf;
	while (1)
	{
		cnt = terminal_read(1, buf, KEYBOARD_BUF_SIZE);
		// terminal_write(1, buf, cnt);
	}
}

/* Tests if terminal_write properly writes data to terminal correctly
 * Inputs	: None
 * Outputs	: None
 * Side Effects	: Testing to see if terminal_write does not stop writing at a null byte
 */
void terminal_test3(){
	int i;
	int32_t cnt;
	char* buf[BUFSIZE];

	// for loop will only go up to 15 characters, meaning rest are null bytes
	for(i = 0; i < 15; i++){
		((char*)buf)[i] = i + 65;		// 65 for start of capital letters
		putc(((char*)buf)[i]);			// print out original 15 characters
	}

	// new line to compare
	putc('\n');
	cnt = terminal_write(1, buf, KEYBOARD_BUF_SIZE);
	printf("%d", cnt);									// prints number of bytes written, should not stop at nulll byte!
}

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */

/* launch_tests
* Description: Test suite entry point
* Inputs: None
* Outputs: None
* Side Effects: Prints messages to screen if helper test involves printing
*/
/* Test suite entry point */
void launch_tests(){
	// launch your tests here
	// terminal_test1();
	// terminal_test2();
	// terminal_test3();
}
