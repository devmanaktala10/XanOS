#ifndef _GUI_STRUCT_H
#define _GUI_STRUCT_H

#include <stdint.h>

#define GUI_IOCTL_ADD_ELEMENT 0
#define GUI_IOCTL_ADD_HANDLER 1
#define GUI_IOCTL_CLEAR_SCREEN 2
#define GUI_IOCTL_DISABLE_SCREEN 3
#define GUI_TERMINAL_COLOR 4
#define GUI_TERMINAL_TEXT 5
#define GUI_TEXT_ELEMENT 6

#define DEFAULT_BUTTON_50_X_50_RED 0
#define DEFAULT_BUTTON_100_X_50_RED 1
#define DEFAULT_BUTTON_25_X_25_RED 2
#define DEFAULT_BUTTON_50_X_25_RED 3

#define DEFAULT_BUTTON_50_X_50_BLUE 4
#define DEFAULT_BUTTON_100_X_50_BLUE 5
#define DEFAULT_BUTTON_25_X_25_BLUE 6
#define DEFAULT_BUTTON_50_X_25_BLUE 7

#define DEFAULT_BUTTON_50_X_50_GRAY 8
#define DEFAULT_BUTTON_100_X_50_GRAY 9
#define DEFAULT_BUTTON_25_X_25_GRAY 10
#define DEFAULT_BUTTON_50_X_25_GRAY 11

#define DEFAULT_BUTTON_50_X_50_YELLOW 12
#define DEFAULT_BUTTON_100_X_50_YELLOW 13
#define DEFAULT_BUTTON_25_X_25_YELLOW 14
#define DEFAULT_BUTTON_50_X_25_YELLOW 15

#define DEFAULT_BOX_BLACK 16
#define DEFAULT_BOX_WHITE 17
#define DEFAULT_BOX_RED 18
#define DEFAULT_BOX_BLUE 19
#define DEFAULT_BOX_GRAY 20
#define DEFAULT_BOX_YELLOW 21
#define DEFAULT_BOX_TRANSPARENT 22

#define COLOR_WHITE 0
#define COLOR_BLACK 1
#define COLOR_RED   2
#define COLOR_GRAY  3
#define COLOR_BLUE  4
#define COLOR_YELLOW 5

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

#endif
