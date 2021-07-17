#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "../types.h"
#include "../lib.h"
#include "i8259.h"
#include "terminal.h"
#include "../syscalls/system_call.h"
#include "graphic_terminal.h"
#include "gui.h"

#define PS2_CONT            0x64
#define PS2_CONT_DATA       0x60
#define READ_CONFIG_CMD     0x20
#define WRITE_CONFIG_CMD    0x60
#define ENABLE_PORT_1       0xAE
#define DISABLE_PORT_1      0xAD
#define ENABLE_PORT_2       0xA8
#define DISABLE_PORT_2      0xA7
#define RESET_KEYBOARD      0xFF
#define ENABLE_SCANNING     0xF4
#define DISABLE_SCANNING    0xF5
#define SET_GET_SCODE       0xF0
#define KEYBOARD_ACK        0xFA
#define KRESET_FAILED_1     0xFC
#define KRESET_FAILED_2     0xFD
#define KTEST_PASSED        0xAA
#define KRESEND_CMD         0xFE
#define K_CMD_TIMEOUT       10000
#define PS2_CONFIG_CLEAN    0xDC
#define SELF_TEST_PS2       0xAA
#define PS2_TEST_PASSED     0x55
#define PORT1_TEST          0xAB
#define PORT1_TEST_PASSED   0x00
#define PORT2_TEST          0xA9
#define KEYBOARD_IRQ           1
#define L_SHIFT_REL         0xAA
#define R_SHIFT_REL         0xB6
#define CTRL_REL            0x9D
#define ALT_REL             0xB9
#define F1                  0x3B
#define F2                  0x3C
#define F3                  0x3D
#define F4                  0x3E
#define F5                  0x3F
#define F6                  0x40
#define F7                  0x41
#define F8                  0x42
#define F9                  0x43
#define F10                 0x44
#define TERMINAL_HEIGHT     425
#define TERMINAL_WIDTH      640

#define MOUSE_ENABLE_DATA 0xF4
#define MOUSE_RESET 0xFF
#define MOUSE_SAMPLE_RATE 0xF3
#define SEND_TO_MOUSE 0xD4
#define MOUSE_IRQ 12

// typedef struct mouse_args{

//     uint8_t byte0;
//     uint8_t byte1;
//     uint8_t byte2;

// } mouse_args_t;
/* Resets PS2 Keyboard Helper */
uint8_t send_cmd_keyboard(uint8_t cmd, unsigned response);

/* Initialize PS2 controller */
void setup_ps2_controller();

/* Interrupt handler for keyboard */
extern int keyboard__interrupt_handler();

int mouse_interrupt_handler();

/* interrupt wrapper in assmebly */
extern void keyboard_helper();

extern void init_mouse();

extern void mouse_helper();

/* keyboard interrupt var */
volatile int keyint;
volatile int prevterm;
volatile int pos_x;
volatile int pos_y;
volatile int click_left;

volatile int icon_exec;
volatile char exec_path[60];

#endif
