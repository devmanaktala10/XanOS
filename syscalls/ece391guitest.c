#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"
#include "ece391guistructs.h"

void button_play(uint32_t button_id);
void song_select(uint32_t box_id);
uint8_t song_selected = 0;
int song_array[7];
int32_t sb_fd;

int main ()
{
    int32_t gui_fd = ece391_open((uint8_t*)"gui");
    sb_fd = ece391_open((uint8_t*)"sb16");

    ioctl(gui_fd, GUI_IOCTL_CLEAR_SCREEN, 0);
    ioctl(gui_fd, GUI_IOCTL_DISABLE_SCREEN, 0);
    ioctl(gui_fd, GUI_TERMINAL_COLOR, COLOR_BLUE);
    ioctl(gui_fd, GUI_TERMINAL_TEXT, COLOR_WHITE);

    gui_user_element_t button;
    button.x = 320;
    button.y = 340;
    button.flags = DEFAULT_BUTTON_50_X_50_GRAY;
    int32_t button_id = ioctl(gui_fd, GUI_IOCTL_ADD_ELEMENT, (unsigned long)(&button));

    gui_handler_addition_t handler;
    handler.handler = (void *)button_play;
    handler.id = button_id;
    int32_t ret_val = ioctl(gui_fd, GUI_IOCTL_ADD_HANDLER, (unsigned long)(&handler));

    if(ret_val == -1){
        ece391_fdputs(1, (uint8_t *)"Handler Addition Failed");
        return 1;
    }

    int i;
    for(i=0; i < 7;i++){
    gui_user_element_t box;
    box.x = 10;
    box.y = 10 + 22*i;
    box.width = 620;
    box.height = 20;
    box.flags = DEFAULT_BOX_GRAY;
    int32_t box_id = ioctl(gui_fd, GUI_IOCTL_ADD_ELEMENT, (unsigned long)(&box));
    song_array[i] = box_id;
    handler.handler = (void *)song_select;
    handler.id = box_id;
    ret_val = ioctl(gui_fd, GUI_IOCTL_ADD_HANDLER, (unsigned long)(&handler));

    if(ret_val == -1){
        ece391_fdputs(1, (uint8_t *)"Handler Addition 2 Failed");
        return 1;
    }

    }

    gui_text_addition_t text;
    text.x = 5;
    text.y = 2;
    text.text = "Shooting Stars";
    text.id = song_array[0];
    text.flags = COLOR_BLACK;

    ret_val = ioctl(gui_fd, GUI_TEXT_ELEMENT, (unsigned long)(&text));

    if(ret_val == -1){
        ece391_fdputs(1, (uint8_t *)"Text Addition Failed");
        return 1;
    }

    text.text = "Star Boy";
    text.id = song_array[1];
    
    ret_val = ioctl(gui_fd, GUI_TEXT_ELEMENT, (unsigned long)(&text));

    if(ret_val == -1){
        ece391_fdputs(1, (uint8_t *)"Text Addition Failed");
        return 1;
    }

    text.text = "A Sky Full Of Stars";
    text.id = song_array[2];
    
    ret_val = ioctl(gui_fd, GUI_TEXT_ELEMENT, (unsigned long)(&text));

    if(ret_val == -1){
        ece391_fdputs(1, (uint8_t *)"Text Addition Failed");
        return 1;
    }

    text.text = "Spaceman";
    text.id = song_array[3];
    
    ret_val = ioctl(gui_fd, GUI_TEXT_ELEMENT, (unsigned long)(&text));

    if(ret_val == -1){
        ece391_fdputs(1, (uint8_t *)"Text Addition Failed");
        return 1;
    }

    text.text = "Space Bound";
    text.id = song_array[4];
    
    ret_val = ioctl(gui_fd, GUI_TEXT_ELEMENT, (unsigned long)(&text));

    if(ret_val == -1){
        ece391_fdputs(1, (uint8_t *)"Text Addition Failed");
        return 1;
    }

    text.text = "Fly Me To The Moon";
    text.id = song_array[5];
    
    ret_val = ioctl(gui_fd, GUI_TEXT_ELEMENT, (unsigned long)(&text));

    if(ret_val == -1){
        ece391_fdputs(1, (uint8_t *)"Text Addition Failed");
        return 1;
    }

    text.text = "All The Stars";
    text.id = song_array[6];
    
    ret_val = ioctl(gui_fd, GUI_TEXT_ELEMENT, (unsigned long)(&text));

    if(ret_val == -1){
        ece391_fdputs(1, (uint8_t *)"Text Addition Failed");
        return 1;
    }

    while(1);

}

void button_play(uint32_t button_id){

    char buffer[32];
    
    switch(song_selected){
        case 0: return;

        case 1: 
                ioctl(sb_fd, 0, (unsigned long)"/music/shootingstars.wav");
                break;

        case 2: 
                ioctl(sb_fd, 0, (unsigned long)"/music/starboy.wav");
                break;

        case 3: 
                ioctl(sb_fd, 0, (unsigned long)"/music/skyfullofstars.wav");
                break;

        case 4:
                ioctl(sb_fd, 0, (unsigned long)"/music/spaceman.wav");
                break;

        case 5: 
                ioctl(sb_fd, 0, (unsigned long)"/music/spacebound.wav");
                break;

        case 6:
                ioctl(sb_fd, 0, (unsigned long)"/music/flymetothemoon.wav");
                break;

        case 7:
                ioctl(sb_fd, 0, (unsigned long)"/music/allthestars.wav");
                break;

        default: return;
    }

}

void song_select(uint32_t box_id){
    
        if(box_id == song_array[0]) song_selected = 1;
        else if(box_id == song_array[1]) song_selected = 2;
        else if(box_id == song_array[2]) song_selected = 3;
        else if(box_id == song_array[3]) song_selected = 4;
        else if(box_id == song_array[4]) song_selected = 5;
        else if(box_id == song_array[5]) song_selected = 6;
        else if(box_id == song_array[6]) song_selected = 7;
        else song_selected = 0;

}
