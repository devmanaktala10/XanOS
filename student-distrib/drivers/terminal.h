#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "../types.h"
#include "../lib.h"
#include "../filesystem/filesystem.h"
#include "keyboard.h"
#include "../paging/page.h"

#define MAX_READ 128
#define CURSOR_COMMAND_PORT 0x3D4
#define CURSOR_DATA_PORT    0x3D5
#define NEWLINE '\n'
#define LOWER_BYTE_COMMAND 0x0F
#define UPPER_BYTE_COMMAND 0x0E
#define BACKSPACE 1
#define NUM_TERMINALS 4

typedef struct terminal {
    int screen_x;
    int screen_y;
    char read[MAX_READ];
    volatile int read_mode;
    volatile int buffer_counter;
    int executed;
} terminal_t;

char args[128];
uint32_t arg_counter;
uint32_t num_args;

terminal_t *current;
int curterm;
terminal_t terminals[NUM_TERMINALS];
unsigned num_terminals;

void write_char(char input);
void set_current(terminal_t * terminal);
void init_terminal(terminal_t * terminal);
void scroll_up(terminal_t * cur, int active);
void set_cursor();
int32_t read_buffer(void * buffer, int32_t nbytes);
int32_t write_terminal(const void * buf, int32_t nbytes);
int32_t terminal_open();
int32_t terminal_close();
int32_t printft(int8_t * format, ...);
void write_string(char * string);
uint32_t set_args(uint8_t* buf, int32_t length);
void reset_term();
int switch_terminal(int data);
void write_char_inactive(char input);
void write_char_current(char input);
void clear_vidmem();
void redraw_screen(int data);
void swap_terminal_pages(int cterm, int nterm);

#endif
