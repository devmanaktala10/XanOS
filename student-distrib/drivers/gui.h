#ifndef _GUI_H
#define _GUI_H

#include "../lib.h"
#include "../types.h"
#include "cirrus.h"
#include "../filesystem/ext2.h"
#include "../paging/page.h"
#include "graphic_terminal.h"
#include "../processes/process.h"
#include "keyboard.h"
#include "../text.h"

#define GUI_PAGE 100
#define GUI_PAGE_BASE_ADDRESS (100*4*1024*1024)

#define STATUSBARID 11
#define ROOTID       0

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 1024
#define BACKGROUNDFBA (4*1024*1024-1)

#define STATUSBAR_HEIGHT 25
#define STATUSBAR_WIDTH 1280
#define STATUSBAR_Y (1024 - 35)
#define STATUSBARFBA (4*1024*1024-1 - 16000 - 1280*25)

#define GUI_BITMAP_LENGTH 30
#define ALLOC_START       (100*1024*1024*4)
#define MAX_GUI_ELEMENTS  240

#define DRAWINGBOARDFBA 1024*1280

#define COLOR_EXPAND_WHITE 0xFEFEFEFE
#define COLOR_EXPAND_BLACK 0xFFFFFFFF
#define COLOR_EXPAND_RED 0x43434343
#define COLOR_EXPAND_GRAY 0x44444444
#define COLOR_EXPAND_BLUE 0x46464646
#define COLOR_EXPAND_YELLOW 0x48484848

#define COLOR_WHITE 0
#define COLOR_BLACK 1
#define COLOR_RED   2
#define COLOR_GRAY  3
#define COLOR_BLUE  4
#define COLOR_YELLOW 5

#define CURSOR_SIZE 32
#define MOUSEFBA 3*1024*1024

#define BAR_WIDTH 640
#define BAR_HEIGHT 25
#define BARFBA (4*1024*1024-1-16000)

#define TERMINAL_TEXT_WIDTH 384
#define TERMINAL_TEXT_HEIGHT 16
#define TERMINAL_TEXT_X 128
#define TERMINAL_TEXT_Y 4

#define CROSS_X 10
#define CROSS_Y 0
#define CROSS_WIDTH 15
#define CROSS_HEIGHT 25

#define MIN_X 25
#define MIN_Y 0 
#define MIN_WIDTH 15
#define MIN_HEIGHT 25

#define GRAPHICAL_TERMINAL_TITLE_OFFSET 32000

#define TYPE_TERMINAL 0x02
#define TYPE_BACKGROUND 0x01
#define TYPE_CLICKABLE 0x04
#define TYPE_TEXT 0x80
#define TYPE_ELEMENT 0x10
#define TYPE_INVISIBLE 0x20
#define TYPE_USER 0x40
#define TYPE_LOADED 0x08
#define TYPE_QUEUED 0x100

#define RENDER_OFF 0x00
#define RENDER_DEFAULT 0x01
#define RENDER_FUNC 0x02
#define RENDER_LOADED 0x10

#define FLAG_IS_RENDERED 0x01

#define USER_DEFAULT_BUTTON_50_X_50_RED 0
#define USER_DEFAULT_BUTTON_100_X_50_RED 1
#define USER_DEFAULT_BUTTON_25_X_25_RED 2
#define USER_DEFAULT_BUTTON_50_X_25_RED 3

#define USER_DEFAULT_BUTTON_50_X_50_BLUE 4
#define USER_DEFAULT_BUTTON_100_X_50_BLUE 5
#define USER_DEFAULT_BUTTON_25_X_25_BLUE 6
#define USER_DEFAULT_BUTTON_50_X_25_BLUE 7

#define USER_DEFAULT_BUTTON_50_X_50_GRAY 8
#define USER_DEFAULT_BUTTON_100_X_50_GRAY 9
#define USER_DEFAULT_BUTTON_25_X_25_GRAY 10
#define USER_DEFAULT_BUTTON_50_X_25_GRAY 11

#define USER_DEFAULT_BUTTON_50_X_50_YELLOW 12
#define USER_DEFAULT_BUTTON_100_X_50_YELLOW 13
#define USER_DEFAULT_BUTTON_25_X_25_YELLOW 14
#define USER_DEFAULT_BUTTON_50_X_25_YELLOW 15

#define USER_DEFAULT_BOX_BLACK 16
#define USER_DEFAULT_BOX_WHITE 17
#define USER_DEFAULT_BOX_RED 18
#define USER_DEFAULT_BOX_BLUE 19
#define USER_DEFAULT_BOX_GRAY 20
#define USER_DEFAULT_BOX_YELLOW 21

#define TERMINALICONFBA (4*1024*1024-1-16000 - 1280*25 - 75*75)
#define COUNTERICONFBA (4*1024*1024-1-16000 - 1280*25 - 2*75*75)
#define FISHICONFBA (4*1024*1024-1-16000 - 1280*25 - 4*75*75)
#define PINGPONGICONFBA (4*1024*1024-1-16000 - 1280*25 - 3*75*75)
#define FONTDATAFBA (4*1024*1024-1-16000 - 1280*25 - 4*75*75 - 256*16)

#define D50X50RED (4*1024*1024-1-16000 - 1280*25 - 4*75*75 - 256*16 - 50*50)
#define D50X50BLUE (4*1024*1024-1-16000 - 1280*25 - 4*75*75 - 256*16 - 2*50*50)
#define D50X50GRAY (4*1024*1024-1-16000 - 1280*25 - 4*75*75 - 256*16 - 3*50*50)
#define D50X50YELLOW (4*1024*1024-1-16000 - 1280*25 - 4*75*75 - 256*16 - 4*50*50)

#define D100X50RED (4*1024*1024-1-16000 - 1280*25 - 4*75*75 - 256*16 - 4*50*50 - 100*50)
#define D100X50BLUE (4*1024*1024-1-16000 - 1280*25 - 4*75*75 - 256*16 - 4*50*50 - 2*100*50)
#define D100X50GRAY (4*1024*1024-1-16000 - 1280*25 - 4*75*75 - 256*16 - 4*50*50 - 3*100*50)
#define D100X50YELLOW (4*1024*1024-1-16000 - 1280*25 - 4*75*75 - 256*16 - 4*50*50 - 4*100*50)

#define D25X25RED (4*1024*1024-1-16000 - 1280*25 - 4*75*75 - 256*16 - 4*50*40 - 4*100*50 - 25*25)
#define D25X25BLUE (4*1024*1024-1-16000 - 1280*25 - 4*75*75 - 256*16 - 4*50*50 - 4*100*50 - 2*25*25)
#define D25X25GRAY (4*1024*1024-1-16000 - 1280*25 - 4*75*75 - 256*16 - 4*50*50 - 4*100*50 - 3*25*25)
#define D25X25YELLOW (4*1024*1024-1-16000 - 1280*25 - 4*75*75 - 256*16 - 4*50*50 - 4*100*50 - 4*25*25)

#define D50X25RED (4*1024*1024-1-16000 - 1280*25 - 4*75*75 - 256*16 - 4*50*50 - 4*100*50 - 4*25*25 - 50*25)
#define D50X25BLUE (4*1024*1024-1-16000 - 1280*25 - 4*75*75 - 256*16 - 4*50*50 - 4*100*50 - 4*25*25 - 2*50*25)
#define D50X25GRAY (4*1024*1024-1-16000 - 1280*25 - 4*75*75 - 256*16 - 4*50*50 - 4*100*50 - 4*25*25 - 3*50*25)
#define D50X25YELLOW (4*1024*1024-1-16000 - 1280*25 - 4*75*75 - 256*16 - 4*50*50 - 4*100*50 - 4*25*25 - 4*50*25)

typedef struct gui_user_element {

    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
    uint32_t flags;

} gui_user_element_t;

typedef struct gui_handler_addition {

    int id;
    void * handler;

} gui_handler_addition_t;

typedef struct gui_text_addition {

    int id;
    char * text;
    uint32_t x;
    uint32_t y;
    uint32_t flags;

} gui_text_addition_t;

typedef struct gui_element {

    uint8_t id;
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
    uint32_t fb_address;
    struct gui_element * parent;
    struct gui_element * elements;
    struct gui_element * children;
    struct gui_element * elements_tail;
    struct gui_element * children_tail;
    uint8_t num_children;
    uint8_t num_elements;
    char fname[60];
    uint16_t type;
    uint8_t render;
    void (*render_func)(void * element);
    void (*click_handler)(void * element, void * mouse_args);
    struct gui_element * next;
    struct gui_element * prev;
    void * user_handler;

} gui_element_t;

gui_element_t gui_root;
gui_element_t status_root;
char gui_alloc_bitmap[30];
int non_terminal_id;

void gui_init();
gui_element_t * alloc_gui_element();
void free_gui_element(gui_element_t * block);
gui_element_t * find_element_by_id(gui_element_t * head, int id);
void remove_element(gui_element_t ** head, gui_element_t ** tail, int id);
void append_element(gui_element_t ** head, gui_element_t ** tail, gui_element_t * elem);
void render_gui();
gui_element_t* check_click_on_elem(int x, int y);
void bring_front(gui_element_t ** head, gui_element_t ** tail, int term);
void terminal_element_initiliazer(gui_element_t * t, int term);
void render_terminal_text(void * element);
void destroy_terminal(int term);
void click_terminal_handler(void * element, void * mouse_args);
void click_cross_handler(void * element, void * mouse_args);
void click_bar_handler(void * element, void * mouse_args);
void click_min_handler(void * element, void * mouse_args);
void render_status(void * element);
int32_t gui_ioctl(unsigned long cmd, unsigned long arg);
int32_t gui_open();
int32_t gui_close();
int32_t gui_read(void * buf, int32_t length);
int32_t gui_write(const void * buf, int32_t length);
void user_click_handler(gui_element_t * t);
void execute_gui_handlers();
void load_gui_assets();
void render_element(gui_element_t * element);
int32_t gui_ioctl_make_element(unsigned long arg);
int32_t gui_ioctl_add_handler(unsigned long arg);
void render_default_element(gui_element_t * element);
void render_default_user_element(gui_element_t * element);
void gui_handler_helper(gui_element_t * t, ptregs_t * temp_ptregs);
void click_handler_helper(gui_element_t * t, mouse_args_t * mouse_args);
void remove_element_no_free(gui_element_t ** head, gui_element_t ** tail, int id);
void remove_user_elements(int term);
void render_desktop_icon(gui_element_t * element);
void desktop_icon_click_handler(void * element, void * mouse_args);
int32_t gui_ioctl_clear_terminal(unsigned long arg);
int32_t init_default_helper(gui_element_t * terminal, gui_user_element_t * elem);
int32_t init_default_50x50(gui_element_t * terminal, gui_user_element_t * e);
int32_t init_default_100x50(gui_element_t * terminal, gui_user_element_t * e);
int32_t init_default_25x25(gui_element_t * terminal, gui_user_element_t * e);
int32_t init_default_50x25(gui_element_t * terminal, gui_user_element_t * e);
void render_default_user_box(gui_element_t * e);
void render_user_text(gui_element_t * t);
void render_status_element(gui_element_t * element);
void status_click_handler(void * element, void * mouse_args);
int32_t init_transparent_box_helper(gui_element_t* terminal, gui_user_element_t* e);

#endif
