#ifndef GRAPHIC_TERMINAL_H
#define GRAPHIC_TERMINAL_H

#include "../types.h"
#include "../lib.h"
#include "../filesystem/filesystem.h"
#include "gui.h"
#include "keyboard.h"
#include "../paging/page.h"
#include "../text.h"
#include "cirrus.h"

#define MAX_READ 128
#define FONT_DATA_START_OFFSET 1280*1024*2
#define TERMINAL_0_START_OFFSET 1280*1024*2
#define TERMINAL_SIZE 32*1024
#define NEWLINE '\n'
#define BACKSPACE 1
#define NUM_GRAPH_TERMINALS 10

typedef struct graphic_terminal {
    int screen_x;
    int screen_y;
    char read[MAX_READ];
    volatile int read_mode;
    volatile int buffer_counter;
    int executed;
    int vidmapped;
    int disabled;
    uint32_t background;
    uint32_t text;
    int r1_mode;
} graphic_terminal_t;

char g_args[128];
uint32_t g_arg_counter;

uint16_t terminal_bitmap;

graphic_terminal_t gterminals[NUM_GRAPH_TERMINALS];

int32_t graphic_terminal_open();
void write_char_graphic_helper(char input);
void write_char_graphic(char input, int terminal);
int is_null_graphic(int x, int y, int terminal);
void scroll_up_graphic(int terminal);
void redraw_screen_graphic();
int32_t read_graphic_terminal(void * buffer, int32_t nbytes);
int32_t write_graphic_terminal(const void * buf, int32_t nbytes);
uint32_t set_args_graphic(uint8_t* buf, int32_t length);
int32_t graphic_printf(int8_t * format, ...);
void reset_graphic_term(int term);
int switch_graphic_terminal(int newterm);
void render_graphic_terminal(void * element);
void set_graphic_terminal_title(int term, char * title);
void free_terminal (uint32_t index);
int find_free_terminal();
void set_terminal_bitmap(uint32_t index, int val);


// void set_cursor();
// int switch_terminal(int data);
// void clear_vidmem();
// void redraw_screen(int data);
// void swap_terminal_pages(int cterm, int nterm);

#endif
