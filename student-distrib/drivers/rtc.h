#ifndef _RTC_H
#define _RTC_H

#include "../types.h"
#include "../lib.h"
#include "i8259.h"
#include "terminal.h"

#define RTC_REGC 0x0C
#define RTC_REGB 0x8B
#define RTC_REGA 0x8A
#define REGISTER_PORT 0x70
#define RW_PORT 0x71
#define BIT6_MASK 0x40
#define BIT8_MASK 0x7F
#define MASK_4 0xF0
#define OPEN_FREQ 2
#define RTC_RATE 32768
#define RTC_MAX_RATE 15
#define RTC_IRQ 8
#define RTC_NMI 0x80
#define NUM_FREQ 10

void rtc_init(void);
void rtc_interrupt_handler();
void rtc_helper();
int32_t rtc_open(void* buf, int32_t length);
int32_t rtc_write(const void* buf, int32_t nbytes);
int32_t write_rtc(uint32_t frequency);
int32_t rtc_read();
int32_t rtc_close();
int32_t rtc_ioctl(unsigned long cmd, unsigned long arg);
extern void rtc_helper();

#endif
