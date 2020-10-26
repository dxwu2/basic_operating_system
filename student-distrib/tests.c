#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "filesystem.h"

#define PASS 1
#define FAIL 0

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

/* Paging Tests */
/* Paging Test 1 - tests that inaccessible locations are inaccessible
 * Inputs	:	None
 * Outputs	:	None
 * Side Effects	:	Freeze the kernel
 */

void paging_test(){
	TEST_HEADER;

	int * ptr = (int*) (0xB8000);
	int a;
	a = *(ptr);
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

/*fs_test_1() - Tests if helper function can read a directory entry by name
 * 
 * Inputs	: None
 * Outputs	: None
 * Coverage : read_dentry_by_name()
 * Side Effects	: Prints PASS if we fetch correct dentry
 */
int fs_test_1(){
	TEST_HEADER;

	dentry_t test_dentry;
	read_dentry_by_name("frame0.txt", &test_dentry);
	if(strncmp(test_dentry.filename, "frame0.txt", 10) != 0)
		return FAIL;
	/* "frame0.txt" should correspond to inode 38 and filetype 2*/
	if(test_dentry.inode_num != 38 || test_dentry.filetype != 2)
		return FAIL;	
	else
		return PASS;
	
}

/*fs_test_2() - Tests if helper function can read a directory entry by index
 * 
 * Inputs	: None
 * Outputs	: None
 * Coverage : read_dentry_by_index()
 * Side Effects	: Prints PASS if we fetch correct dentry
 */
int fs_test_2(){
	TEST_HEADER;

	dentry_t test_dentry;
	read_dentry_by_index(11, &test_dentry);
	printf("filename: %s", test_dentry.filename);
	if(strncmp(test_dentry.filename, "counter", 7) != 0)
		return FAIL;
	if(test_dentry.inode_num != 42 || test_dentry.filetype != 2)
		return FAIL;
	else
		return PASS;
	
}

/* fs_test_list_files - Tests dir_read (and therefore, read_dentry_by_name) by printing out all the files
 * 
 * Inputs	: None
 * Outputs	: None
 * Coverage	: dir_read and read_dentry_by_name
 * Side Effects	: lists out all files (check discord general chat for what it should look like)
 */
void fs_test_list_files(){
	TEST_HEADER;

	dentry_t cur_dentry;
	int i;
	for (i = 0; i < 17; i++) {
		dir_read(i, &cur_dentry, 64);
		uint32_t inode_idx = cur_dentry.inode_num;
		inode_t* inode_addr = filesystem_start + (4096 * (inode_idx+1)); //calculate inode struct addr using inode number and start addr of filesystem
		// if(cur_dentry.filename > FILENAME_LEN)
        // strncpy(cur_dentry.filename, cur_dentry.filename, 32);
		printf("file_name: %s, file_type: %d, file_size: %d\n", cur_dentry.filename, cur_dentry.filetype, inode_addr->length);
	}
	
}

/* fs_test_read_file - Tests file_read (and thus, read_data) by printing out contents of a file
 * 
 * Inputs	: None
 * Outputs	: None
 * Coverage	: file_read and read_data
 * Side Effects	: prints out the contents of the specified file
 */
void fs_test_read_file(){
	TEST_HEADER;

	uint32_t fd;		// unused here
	char buffer[1600];	// more than enough
	file_open((uint8_t*)"frame0.txt");
	file_read((uint32_t)&fd, &buffer, 1600);
	int i;
	for (i = 0; i < 1600; i++) {
		printf("%c", buffer[i]);
	}
	printf("\nfile_name: frame1.txt");
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
	// TEST_OUTPUT("idt_test", idt_test());
	// divide_by_zero_test();
	// system_call_test();
	// paging_test1();
	// null_test();
	// TEST_OUTPUT("fs_test_1", fs_test_1());
	// TEST_OUTPUT("fs_test_2", fs_test_2());
	fs_test_list_files();
	// fs_test_read_file();
}
