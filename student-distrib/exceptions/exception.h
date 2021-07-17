#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#include "../types.h"
#include "../syscalls/system_call.h"
#include "../lib.h"
#include "../x86_desc.h"

#define NUM_EXCEPTIONS 20
#define RESERVED_EXCEPTION 15

/* Exception Functions/Handlers */
void load_exceptions();
void exception_handler(uint32_t offset);
extern void divide_by_zero_handler();
extern void debug_handler();
extern void NMI_handler();
extern void breakpoint_handler();
extern void overflow_handler();
extern void bound_range_handler();
extern void invalid_opcode_handler();
extern void device_NA_handler();
extern void double_fault_handler();
extern void coproccesor_segment_handler();
extern void invalid_TSS_handler();
extern void segment_NP_handler();
extern void stack_fault_handler();
extern void general_protection_handler();
extern void page_fault_handler();
extern void FP87_handler();
extern void alignment_handler();
extern void machine_check_handler();
extern void FPSIMD_handler();

#endif
