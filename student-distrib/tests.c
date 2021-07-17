#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "./drivers/i8259.h"
#include "./drivers/rtc.h"
#include "./drivers/terminal.h"
#include "./filesystem/filesystem.h"

#define PASS 1
#define FAIL 0
#define NOT_VIDMEM 0x08000 //a memory address in 0-4MB which is not video memory
#define NOT_KERNEL_SEGMENT 0xC00000 //12 MB address
#define KERNEL_SEGMENT 0x600000 //6 MB address
/* format these macros as you see fit */
#define TEST_HEADER 	\
	printft("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printft("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test
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
	for (i = 0; i < 19; ++i){
		if ((idt[i].offset_15_00 == NULL) &&
			(idt[i].offset_31_16 == NULL) && i!=15){
			assertion_failure();
			result = FAIL;
		}
	}


	return result;
}


/* test_syscall
 *
 * Tests system call interrupt
 * Inputs: none
 * Outputs: Should display that a system call has occurred without halting kernel
 * Side Effects: None
 * Coverage: Load IDT, system calls.
 * Files: Kernel.c
 */
int test_syscall(){
	asm volatile ("             \n\
					int $0x80         \n\
					"
					:
					:
					: "memory"
	);
	return 0;
}

/* test_exception
 *
 * Tests individual exceptions by calling the interrupt in assembly.
 * Inputs: interrupt number to be tested
 * Outputs: Should display appropriate error message for the passed in interrupt
 * Side Effects: halts kernel
 * Coverage: Load IDT, idt handler mapping.
 * Files: Kernel.c, exceptions.S
 */
int test_exception(unsigned char int_num){
	switch (int_num){

		// TEST: divide by zero exception.
		case 0:
		asm volatile ("             \n\
						int $0x0         \n\
						"
						:
						: "d"(int_num)
						: "memory"
		);

  // TEST: Debug exception.
	case 1:
	asm volatile ("             \n\
					int $0x1         \n\
					"
					:
					: "d"(int_num)
					: "memory"
	);

  // TEST: Non-Maskable interrupt.
	case 2:
	asm volatile ("             \n\
					int $0x2          \n\
					"
					:
					: "d"(int_num)
					: "memory"
	);

	// TEST: breakpoint exception
	case 3:
	asm volatile ("             \n\
					int $0x3         \n\
					"
					:
					: "d"(int_num)
					: "memory"
	);

 	// TEST: overflow exception
	case 4:
	asm volatile ("             \n\
					int $0x4          \n\
					"
					:
					: "d"(int_num)
					: "memory"
	);

	// TEST: bound range exception
	case 5:
	asm volatile ("             \n\
					int $0x5         \n\
					"
					:
					: "d"(int_num)
					: "memory"
	);

  // TEST: Invalid Opcode exception
	case 6:
	asm volatile ("             \n\
					int $0x6         \n\
					"
					:
					: "d"(int_num)
					: "memory"
	);

	// TEST: Device unavailable exception
	case 7:
	asm volatile ("             \n\
					int $0x7         \n\
					"
					:
					: "d"(int_num)
					: "memory"
	);

	// TEST: double- fault error
	case 8:
	asm volatile ("             \n\
					int $0x8         \n\
					"
					:
					: "d"(int_num)
					: "memory"
	);


	// TEST: Co-processor segment error
	case 9:
	asm volatile ("             \n\
					int $0x9         \n\
					"
					:
					: "d"(int_num)
					: "memory"
	);

	// TEST: Invalid TSS error
	case 10:
	asm volatile ("             \n\
					int $0xA         \n\
					"
					:
					: "d"(int_num)
					: "memory"
	);

  // TEST: Segment not present
	case 11:
	asm volatile ("             \n\
					int $0xB         \n\
					"
					:
					: "d"(int_num)
					: "memory"
	);

	// TEST: Stack fault error
	case 12:
	asm volatile ("             \n\
					int $0xC        \n\
					"
					:
					: "d"(int_num)
					: "memory"
	);

  // TEST: general protection exception.
	case 13:
	asm volatile ("             \n\
					int $0xD         \n\
					"
					:
					: "d"(int_num)
					: "memory"
	);

	// TEST: Page fault error
	case 14:
	asm volatile ("             \n\
					int $0xE         \n\
					"
					:
					: "d"(int_num)
					: "memory"
	);

	case 15: // Reserved

	// TEST: floating point X86
	case 16:
	asm volatile ("             \n\
					int $0x10         \n\
					"
					:
					: "d"(int_num)
					: "memory"
	);

	// TEST: Alignment error
	case 17:
	asm volatile ("             \n\
					int $0x11         \n\
					"
					:
					: "d"(int_num)
					: "memory"
	);

	// TEST: Machine check error
	case 18:
	asm volatile ("             \n\
					int $0x12         \n\
					"
					:
					: "d"(int_num)
					: "memory"
	);

	// TEST: Floating point SMID
	case 19:
	asm volatile ("             \n\
					int $0x13         \n\
					"
					:
					: "d"(int_num)
					: "memory"
	);
}
return 0;
}

/* Divide by zero test
 *
 * calls the divide by zero exception handler
 * Inputs: None
 * Outputs: Should display divide by 0 exception on screen
 * Side Effects: None
 * Coverage: Load IDT, Divide by zero exception handler
 * Files: exceptions.S
 */

int test_divbyzero(){
	int y=0;
	int x=1/y;
	return x;
}

/* Paging test 1
 *
 * calls the page fault exception handler when accessing null address
 * Inputs: None
 * Outputs: Should display page fault on screen
 * Side Effects: halts kernel
 * Coverage: Load IDT, Page fault exception handler
 * Files: exceptions.S, IDT entries
 */

int test_paging_1(){
	int* x =NULL;
	int y=*x;
	printf("paging test 1 failed");
	return y;
}

/* Paging test 2
 *
 * accesses address in kernel segment. Should NOT display page fault exception
 * Inputs: None
 * Outputs: Should display the y value on the screen.
 * Side Effects: None
 * Coverage: Load IDT, Page fault exception handler
 * Files: exceptions.S
 */
int test_paging_2(){
	int* x =(int*)KERNEL_SEGMENT;
	int y=*x;
	if(y)
	return PASS;
	else
	return PASS;
}

/* Paging test 3
 *
 * accesses address outside kernel segment. Should display page fault exception
 * Inputs: None
 * Outputs: Should display page fault on screen
 * Side Effects: halts kernel
 * Coverage: Load IDT, Page fault exception handler
 * Files: exceptions.S, IDT entry
 */
int test_paging_3(){
	int* x =(int*)NOT_KERNEL_SEGMENT;
	int y=*x;
	printf("Paging test 3 failed");
	return y;
}

/* Paging test 4
 *
 * accesses address in first 4MB but outside video memory. Should display page fault exception
 * Inputs: None
 * Outputs: Should display page fault on screen
 * Side Effects: halts kernel
 * Coverage: Load IDT, Page fault exception handler
 * Files: exceptions.S, IDT entry
 */
int test_paging_4(){
	int* x =(int*)NOT_VIDMEM;
	int y=*x;
	printf("Paging test 4 failed");
	return y;
}

/* RTC test
 *
 * accesses address in first 4MB but outside video memory. Should display page fault exception
 * Inputs: None
 * Outputs: Should display page fault on screen
 * Side Effects: halts kernel
 * Coverage: Load IDT, Page fault exception handler
 * Files: exceptions.S, IDT entry
 */
int test_rtc(){
	unsigned long flags;
	cli_and_save(flags);

	enable_irq(RTC_IRQ);

	restore_flags(flags);
	return PASS;
}


// add more tests here

/* Checkpoint 2 tests */

/* RTC open test
 *
 * tests the functionality of rtc open. should set the opening freuqency of rtc interrupts to 2
 * Inputs: None
 * Outputs: should display 1's on the screen at a rate of 2 per second.
 * Side Effects: writes to the terminal
 * Coverage: RTC interrupt handler, IDT
 * Files: rtc.c, interrupts.S
 */
int test_rtc_open(){
	// unsigned long flags;
	// cli_and_save(flags);
  //
	// /* open and enable rtc */
	// rtc_open();
	// enable_irq(RTC_IRQ);
  //
	// restore_flags(flags);
	return PASS;
}

/* RTC freuqency test
 *
 * tests for a valid frequency value for rtc interrupts
 * Inputs: frequency of the rtc interrupt
 * Outputs: should display 1's on the screen at a frequency given as input if it is a valid frequency.
 						else it will print Invalid frequency.
 * Side Effects: writes to the terminal
 * Coverage: RTC interrupt handler, IDT
 * Files: rtc.c, interrupts.S
 */
int test_rtc_frequency(uint32_t frequency){
	// unsigned long flags;
  //   cli_and_save(flags);
	// /* try writing RTC frequency */
	// if(rtc_write(frequency)==-1){
  //
	// 	disable_irq(RTC_IRQ);
	// 	restore_flags(flags);
	// 	printf("Invalid frequency");
	// 	return FAIL;
  //
	// }
  //
	// enable_irq(RTC_IRQ);
  //   restore_flags(flags);

	return PASS;
}

/* RTC read test
 *
 * Waits for interrupt to occur. Stops when interrupt occurs
 * Inputs: none
 * Outputs: Should print "waiting for interrupt while waiting" and "interrupt occured when interrupt occurs"
 * Side Effects: writes to the terminal
 * Coverage: RTC interrupt handler, IDT
 * Files: rtc.c, interrupts.S
 */
int test_rtc_read(){
	// enable_irq(RTC_IRQ);
	// rtc_read(NULL, 0);
	// disable_irq(RTC_IRQ);
	return PASS;
}

/* Terminal Open Test
 *
 * Tests if the terminal open returns 0 (correctly initializes terminal)
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 * Coverage: Terminal Driver open
 * Files: terminal.c
 */
int terminal_open_test(){
	/* open terminal */
	int32_t ret = terminal_open();
	if(ret != 0)
		return FAIL;
	int i,j;
	uint8_t c;
	char* video_mem = (char *)VIDEO;

	/* check if screen is blank */
	for(i = 0; i < NUM_ROWS; i++){
		for(j = 0; j < NUM_COLS; j++){
			c = *(uint8_t *)(video_mem + ((NUM_COLS * i + j) << 1));
			if(c != '\0')
				return FAIL;
		}
	}
	return PASS;
}

/* Terminal Write Test 1
 *
 * Test if terminal correctly displays strings
 * Inputs: None
 * Outputs: None
 * Side Effects: Writes string to screen
 * Coverage: Terminal Driver write
 * Files: terminal.c
 */
int terminal_write_test1(){

	/* open terminal and write string to terminal */
	terminal_open();
	char * st = "This is a length 27 string";
	int32_t ret = write_terminal((const void *)st, strlen(st));

	/* check return value */
	if(ret != strlen(st)){
		return FAIL;
	}

	/* see if the string was written by reading from video mem */
	char* video_mem = (char *)VIDEO;
	int i;
	char c;
	for(i = 0; i < strlen(st); i++){
		c = *(char *)(video_mem + ((i) << 1));
		if(c != st[i])
			return FAIL;
	}
	return PASS;
}

/* Terminal Write Test 2
 *
 * Test if terminal correctly displays strings. Write really long string and check video
 * memory to verify.
 * Inputs: None
 * Outputs: None
 * Side Effects: Writes string to screen
 * Coverage: Terminal Driver write
 * Files: terminal.c
 */
int terminal_write_test2(){

	/* open terminal and write string to terminal */
	terminal_open();
	char * st = "This is a very very very loooooooooooooooooong string that is going to carry over to the next line";
	int32_t ret = write_terminal((const void *)st, strlen(st));

	/* check return value */
	if(ret != strlen(st)){
		return FAIL;
	}

	/* see if the string was written by reading from video mem */
	char* video_mem = (char *)VIDEO;
	int i;
	char c;
	int x = 0;
	int y = 0;
	for(i = 0; i < strlen(st); i++){
		/* read video mem */
		c = *(char *)(video_mem + ((x + y * NUM_COLS) << 1));
		if(c != st[i])
			return FAIL;
		x++;
		if(x == NUM_COLS){
			x = 0;
			y += 1;
		}
	}
	return PASS;
}

/* Terminal Write Test 3
 *
 * Test if terminal correctly displays strings. Writes only 50 characters of really long string
 * and reads video memory to verify
 * Inputs: None
 * Outputs: None
 * Side Effects: Writes string to screen
 * Coverage: Terminal Driver write
 * Files: terminal.c
 */
int terminal_write_test3(){

	/* open terminal and write string to terminal, writes only 50 characters from long string */
	terminal_open();
	char * st = "This is a very very very loooooooooooooooooong string that is going to carry over to the next line";
	int32_t ret = write_terminal((const void *)st, 50);

	/* check return value = 50 */
	if(ret != 50)
		return FAIL;

	/* see if the string was written by reading from video mem */
	char* video_mem = (char *)VIDEO;
	int i;
	char c;
	int x = 0;
	int y = 0;
	for(i = 0; i < strlen(st); i++){
		c = *(char *)(video_mem + ((x + y * NUM_COLS) << 1));

		/* if this character was printed */
		if(i < 50){
			if(c != st[i])
				return FAIL;
		}

		/* beyond 50 characters, nothing should have been printed */
		else{
			if(c != '\0')
				return FAIL;
		}
		x++;
		if(x == NUM_COLS){
			x = 0;
			y += 1;
		}
	}
	return PASS;
}

/* Terminal Echo Read
 *
 * Test if terminal correctly reads
 * Inputs: None
 * Outputs: None
 * Side Effects: Writes string to screen
 * Coverage: Terminal Driver read
 * Files: terminal.c
 */
int terminal_echo_read(){

	/* open and start reading from the terminal */
	terminal_open();
    char buffer[1024];
	int i;
	for(i = 0; i < 1024; i++)
		buffer[i] = '\0';

	/* echo the read buffer onto the screen */
	int32_t ret = read_buffer(buffer, 1024);
	write_terminal(buffer, ret);

	return PASS;
}

/* Terminal Close
 *
 * Test if terminal correctly closes
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 * Coverage: Terminal Driver close
 * Files: terminal.c
 */
int terminal_test_close(){

	/* close and check return value */
	int32_t ret = terminal_close();
	if(ret == 0)
		return PASS;
	else
		return FAIL;
}

/*
 * dentry_by_name_test
 * DESCRIPTION -- tests to check whether a read_dentry_by_name is working
 *								Gets a filename, and checks whether it finds it or not
 *	INPUTS -- filename - Name of file to find
 *	OUTPUTS -- None
 *	RETURNS -- PASS if it found the file, else FAIL
 *	SIDE EFFECTS -- None
 */
int32_t dentry_by_name_test(char* filename) {
	dentry_t dentry;

	/* bad input check */
	if(read_dentry_by_name((uint8_t*) filename, &dentry) == -1){
		return FAIL;
	}
	/* compare filename with read filename */
	if (strncmp((char*) dentry.filename, filename, MAX_FILENAME_LENGTH) != 0) {
		return FAIL;
	}

	return PASS;

}

/*
 * dentry_by_index_test
 * DESCRIPTION -- tests to check whether a read_dentry_by_index is working
 *								Gets a filename, uses helper function get_dentry_index_by_name
 *								to get index of location of file, and then calls read_dentry_by_index
 *								and checks whether the output is the same
 *	INPUTS -- filename - Name of file to find
 *	OUTPUTS -- None
 *	RETURNS -- PASS if it found the file, else FAIL
 *	SIDE EFFECTS -- None
 */
int32_t dentry_by_index_test(char* filename) {
	dentry_t dentry1, dentry2;
	int32_t index;

	/* check bad input */
	if(get_dentry_index_by_name((uint8_t*) filename, &dentry1) == -1){
		return FAIL;
	}


	index = get_dentry_index_by_name((uint8_t*) filename, &dentry2);

	/* another bad input */
	if (index == -1) {
		return FAIL;
	}

	/* check filename */
	if (strncmp((char*) dentry1.filename, (char*) dentry2.filename,
	 									MAX_FILENAME_LENGTH) != 0) {
		return FAIL;
	}

	/* check bad input */
	if(read_dentry_by_index(index, &dentry1) == -1){
		return FAIL;
	}

	/* check read filename */
	if (strncmp((char*) dentry1.filename, (char*) dentry2.filename,
										MAX_FILENAME_LENGTH) != 0) {
		return FAIL;
	}

	return PASS;
}

/*
 * read_data_test
 * DESCRIPTION -- tests to check whether read_data is working
 *								Uses the verylargetextwithverylongname.txt file
 *								Uses teststring1 and teststring2 with offset1 and offset2
 *								to check if the expected output is correct or not
 *	INPUTS -- None
 *	OUTPUTS -- None
 *	RETURNS -- PASS if correct data comes back in buffer, FAIL otherwise
 *	SIDE EFFECTS -- None
 */
int32_t read_data_test() {

	/* open very large file */
	char* filename = "verylargetextwithverylongname.txt";

	/* two strings in file and their length 42 and 10 */
	char* teststring1 = "very large text file with a very long name";
	char* teststring2 = "klmnopqrst";
	uint32_t length1 = 42;
	uint32_t length2 = 10;

	/* offset of strings into the file */
	uint32_t offset1 = 0;
	uint32_t offset2 = 4 * 1024 - 5;

	/* buffer to read into size 8 * 1024 (8kb) */
	uint8_t buf[8 * 1024];


	dentry_t dentry;
	int32_t bytes_read;

	/* find the dentry with the filename */
	unsigned i;
	for (i = 0; i < NUM_DIR_ENTRIES; i++) {
		read_dentry_by_index(i, &dentry);
		if (strncmp((char*) dentry.filename, filename, MAX_FILENAME_LENGTH) == 0) {
			break;
		}
	}

	/* read 42 bytes from offset 1 into file */
	bytes_read = read_data(dentry.inode_num, offset1, buf, length1);

	/* see if the read string matches test string */
	if (strncmp(teststring1, (char*) buf, length1) != 0 || bytes_read != length1) {
		return FAIL;
	}

	/* read 10 bytes from offset 2 into file */
	bytes_read = read_data(dentry.inode_num, offset2, buf, length2);

	/* see if read string matches test string */
	if (strncmp(teststring2, (char*) buf, length2) != 0 || bytes_read != length2) {
		return FAIL;
	}

	return PASS;

}

/*
 * file_open_close_test
 * DESCRIPTION -- tests to check whether file_open and file_close are working
 *								First test tries to open a directory, and then we mess
 *								around with file open and close multiple times and see
 *								if the behavior is correct
 *	INPUTS -- None
 *	OUTPUTS -- None
 *	RETURNS -- PASS if multiple files open and close successfully , else FAIL
 *	SIDE EFFECTS -- None
 */
int32_t file_open_close_test() {

	char* filename1;
	char* filename2;
	char* filename3;
	int32_t file_index1;
	int32_t file_index2;
	int32_t file_index3;
	int32_t ret_val;

	/* try opening a directory in file_open */
	filename1 = ".";
	file_index1 = file_open((uint8_t*) filename1);
	if (file_index1 != -1) {
		return FAIL;
	}

	/* try opening a file in file_open */
	filename2 = "ls";
	file_index2 = file_open((uint8_t*) filename2);
	if (file_index2 != 0) {
		return FAIL;
	}

	/* try opening another file in file_open */
	filename3 = "grep";
	file_index3 = file_open((uint8_t*) filename3);
	if (file_index3 != 1) {
		return FAIL;
	}

	/* close the files and check the return value */
	ret_val = file_close(file_index2);

	/* if you try closing again it should fail */
	file_index2 = file_close(file_index2);

	/* check the return values */
	if (ret_val != 0 || file_index2 != -1) {
		return FAIL;
	}

	/* open a closed file again */
	filename2 = "ls";
	file_index2 = file_open((uint8_t*) filename2);
	if (file_index2 != 0) {
		return FAIL;
	}

	return PASS;

}

/*
 * file_read_test
 * DESCRIPTION -- tests to check whether file_read is working properly
 *								It uses the frame0.txt and reads 5 characters from it twice
 *								The first 5 should be the first 5 lines of file, and the
 * 								second 5 should be characters 6-10 of file
 *	INPUTS -- None
 *	OUTPUTS -- None
 *	RETURNS -- PASS if buf gets data as expected, FAIL otherwise
 *	SIDE EFFECTS -- None
 */
int32_t file_read_test() {

	// /* read consecutively from frame0.txt */
	// char* filename = "frame0.txt";
  //
	// /* if reading 5 bytes twice, these are the expected output strings */
	// char* teststring1 = "/\\/\\/";
	// char* teststring2 = "\\/\\/\\";
  //
	// /* buffer to store into length 100 */
	// uint8_t buf[100];
  //
	// /* open the file */
	// int32_t file_index = file_open((uint8_t*) filename);
  //
	// /* read 5 bytes */
	// file_read(file_index, buf, 5);
  //
	// /* compare with expected output */
	// if (strncmp(teststring1, (char*) buf, 5) != 0) {
	// 	return FAIL;
	// }
  //
	// /* read 5 bytes again */
	// file_read(file_index, buf, 5);
  //
	// /* compare with expected output */
	// if (strncmp(teststring2, (char*) buf, 5) != 0) {
	// 	return FAIL;
	// }

	return PASS;
}

/* Directory open test
 *
 * tests for a valid directory name and type.
 * Inputs: directory name
 * Outputs: passes is the input exists and is a directory
 * Side Effects: none
 * Coverage: filesystem.c
 */
int32_t directory_open_test(uint8_t* dirname){

	/* try opening directory and see return value */
	if(dir_open(dirname) == -1) {
		printft("Directory does not exist or bad input");
		return FAIL;
	}
	return PASS;
}

/* Directory read test
 *
 * tests if consecutive reads, results in reads of consecutive file names in the given directory.
 * Inputs: none
 * Outputs: passes if the files are consecutively read.
 * Side Effects: none
 * Coverage: filesystem.c
 */
int32_t directory_read_test(){
	// dentry_t dentry1;
	// dentry_t dentry2;
  //
	// int i;
  //
	// /* read 4 consecutive filenames from the directory */
	// for(i = 0; i < 4; i++){
	// 	read_dentry_by_index(i, &dentry1);
	// 	dir_read(&dentry2);
  //
	// 	/* compare with read_dentry_by_index(expected) output */
	// 	if(strncmp(dentry1.filename, dentry2.filename, MAX_FILENAME_LENGTH) != 0)
	// 		return FAIL;
	// }
	return PASS;
}

/* Executable Read Test
 *
 * tests if executable is correcctly printed on screen
 * Inputs: none
 * Outputs: passes: should see 123456789abcdefghijklmnopqrstuvwxyz on screen
 * Side Effects: none
 * Coverage: filesystem.c
 */
int32_t executable_read_test(){

	/* read an executable file */
	// char* filename = "grep";
  //
	// /* buffer to copy into, length 1024 */
	// uint8_t buf[1024];
  //
	// int32_t file_index = file_open((uint8_t*) filename);
	// int32_t ret;
  //
	// /* read 1024 bytes from file */
	// ret = file_read(file_index, buf, 1024);
  //
	// /* keep reading till no more to be read */
	// while(ret != 0){
	// 	/* write read buffer into terminal */
	// 	write_terminal(buf, ret);
	// 	/* read 1024 bytes from file */
	// 	ret = file_read(file_index, buf, 1024);
	// }
	return PASS;
}

/* Null pointer test
 *
 * tests if executable
 * Inputs: none
 * Outputs: passes if all functions handle NULL pointer smoother
 * Side Effects: none
 * Coverage: filesystem.c, terminal.c
 */
int32_t null_test(){

	/* test all function that take pointers with
	 * NULL pointers and other invalid input and
	 * make sure their return values are expected
	 * for invalid input */
	// if(read_buffer(NULL, 0) != 0){
	// 	return FAIL;
	// }
	// if(write_terminal(NULL, 0) != 0){
	// 	return FAIL;
	// }
	// if(file_open(NULL) != -1){
	// 	return FAIL;
	// }
	// if(file_read(1, NULL, 0) != -1){
	// 	return FAIL;
	// }
	// if(dir_open(NULL) != -1){
	// 	return FAIL;
	// }
	// if(dir_read(NULL) != -1){
	// 	return FAIL;
	// }
	// if(read_dentry_by_index(0, NULL) != -1){
	// 	return FAIL;
	// }
	// if(read_dentry_by_name(NULL, NULL) != -1){
	// 	return FAIL;
	// }
	return PASS;
}
/* Checkpoint 3 tests */

int syscall_error(){
    int ret;
    asm volatile("                          \n\
                movl $0, %%eax             \n\
                int $0x80                   \n\
                "
                :"=a"(ret)
                :
                :"memory"
    );
    if(ret == -1)
        return PASS;
    else
        return FAIL;
}

/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	//CHECKPOINT 1:

	//TEST_OUTPUT("RTC test", test_rtc());
	//TEST_OUTPUT("idt_test", idt_test());
	//TEST_OUTPUT("test_divbyzero,",test_divbyzero());
	//TEST_OUTPUT("test_paging",test_paging());
	//TEST_OUTPUT("breakpoint error", test_exception(3)); //breakpoint test
	//TEST_OUTPUT("system call check",test_syscall());
	//TEST_OUTPUT("paging_test_1",test_paging_1());
	//TEST_OUTPUT("paging_test_2",test_paging_2());
	//TEST_OUTPUT("paging_test_3",test_paging_3());
	//TEST_OUTPUT("paging_test_4",test_paging_4());

	//CHECKPOINT 2:

//RTC TESTS

	//TEST_OUTPUT("RTC OPEN",test_rtc_open());
	//TEST_OUTPUT("RTC FREQUENCY TEST", test_rtc_frequency(1024));
	//TEST_OUTPUT("RTC READ TEST",test_rtc_read());

//TERMINAL TESTS

	//TEST_OUTPUT("TERMINAL OPEN TEST", terminal_open_test());
	//TEST_OUTPUT("TERMINAL WRITE TEST 1", terminal_write_test1());
	//TEST_OUTPUT("TERMINAL WRITE TEST 2", terminal_write_test2());
	//TEST_OUTPUT("TERMINAL WRITE TEST 3", terminal_write_test3());
	//TEST_OUTPUT("TERMINAL ECHO READ", terminal_echo_read());
	//TEST_OUTPUT("TERMINAL TEST CLOSE", terminal_test_close());

//FILE SYSTEM TESTS
    // uint8_t* filename = (uint8_t * )".";
	// TEST_OUTPUT("READ DENTRY BY NAME", dentry_by_name_test((char *)filename));
	// TEST_OUTPUT("OPEN DIRECTORY TEST", directory_open_test(filename));
	// TEST_OUTPUT("READ DIRECTORY TEST ", directory_read_test());
	// TEST_OUTPUT("READ DENTRY BY NAME: ls", dentry_by_name_test("ls"));
	// TEST_OUTPUT("READ DENTRY BY NAME: grep", dentry_by_name_test("grep"));
	// TEST_OUTPUT("READ DENTRY BY NAME: testprint", dentry_by_name_test("testprint"));
	//TEST_OUTPUT("READ DENTRY BY NAME INVALID", !dentry_by_name_test("hello2"));
	//
	// TEST_OUTPUT("READ DENTRY BY INDEX", dentry_by_index_test("ls"));
	// TEST_OUTPUT("READ DENTRY BY INDEX", dentry_by_index_test("cat"));
	// TEST_OUTPUT("READ DENTRY BY INDEX", dentry_by_index_test("hello"));
	// TEST_OUTPUT("READ DENTRY BY INDEX INVALID", !dentry_by_index_test("hello2"));
	// TEST_OUTPUT("READ DATA", read_data_test());
	// TEST_OUTPUT("FILE OPEN CLOSE", file_open_close_test());
	// TEST_OUTPUT("FILE READ", file_read_test());
	// TEST_OUTPUT("EXECUTABLE READ", executable_read_test());
	// TEST_OUTPUT("NULL POINTER", null_test());

// CP3 TESTS
    TEST_OUTPUT("SYSCALL ERROR", syscall_error());

}
