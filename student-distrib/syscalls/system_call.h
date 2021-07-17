#ifndef _SYSTEM_CALL_H
#define _SYSTEM_CALL_H

#include "../x86_desc.h"
#include "../lib.h"
#include "../filesystem/filesystem.h"
#include "../paging/page.h"
#include "../drivers/rtc.h"
#include "../processes/process.h"
#include "../filesystem/ext2.h"
#include "../signals/signals.h"
#include "../drivers/graphic_terminal.h"
#include "../drivers/gui.h"
#include "../drivers/networking.h"
#include "../text.h"
#include "../drivers/sb16.h"

#define SYS_CALL          0x80
#define MAX_FD_LENGTH     8
#define RTC_FILETYPE      0
#define DENTRY_FILETYPE   1
#define REG_FILETYPE      2

/* Load system call entry into the IDT */
void load_system_call();

extern void guireturn_helper(ptregs_t temp_ptregs);

int32_t halt(uint8_t status);
int32_t execute(const uint8_t* command);
int32_t execute_new_term(const uint8_t * command, uint32_t term);
int32_t read(int32_t fd, void* buf, int32_t nbytes);
int32_t write(int32_t fd, const void* buf, int32_t nbytes);
int32_t open(const uint8_t* filename);
int32_t close(int32_t fd);
int32_t getargs(uint8_t* buf, int32_t nbytes);
int32_t vidmap(uint8_t** screen_start);
int32_t set_handler(int32_t signum, void* handler_address);
int32_t sigreturn(void);
void setup_stdinout();
int32_t chdir(uint8_t * buf);
int32_t mkdir(uint8_t * buf);
int32_t rmdir(uint8_t * buf);
int32_t create(uint8_t * buf);
int32_t unlink(uint8_t * buf);
int32_t ioctl(int32_t fd, unsigned long cmd, unsigned long arg);
int32_t guireturn(void);

/* asm function to run process */
int32_t execute_asm(uint32_t eip, uint32_t cs, uint32_t esp, uint32_t ss);
/* syscall_handler */
int32_t syscall_handler();
void kill_terminal(int term);

#endif
