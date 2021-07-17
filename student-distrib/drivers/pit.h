#ifndef _PIT_H
#define _PIT_H

#include "../types.h"
#include "../x86_desc.h"
#include "../lib.h"
#include "terminal.h"
#include "i8259.h"
#include "graphic_terminal.h"
#include "gui.h"
#include "keyboard.h"

#define CH0_DATA_PORT 0x40
#define SET_MODE 0x34
#define MODE_REG 0x43
#define SET_FREQ 0xFF //50ms

void pit_init();
void pit_handler();
extern void pit_helper();

#endif
