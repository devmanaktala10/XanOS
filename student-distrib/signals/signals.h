#ifndef _SIGNALS_H
#define _SIGNALS_H

#include "../x86_desc.h"
#include "../processes/process.h"
#include "../syscalls/system_call.h"
#include "../drivers/graphic_terminal.h"

uint32_t tss_args[3];
int32_t sig_default_action(int32_t signum);
void kill_process();
void ignore_sig();
void set_tss_args();
void execute_signals();
void sig_desc(int32_t signum);
// void signal_handler();

#endif
