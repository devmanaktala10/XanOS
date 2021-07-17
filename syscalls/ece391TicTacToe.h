#ifndef _TICTACTOE_H
#define _TICTACTOE_H

#define X_WIDTH  640
#define Y_HEIGHT 400
#define BUFSIZE  128

uint8_t cross[800];
uint8_t circle[800];
int game_arr[9] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
char sender_port[6];
char udp_port[6];
int box_id[9];
volatile int waiting;
volatile int disable_handler;

int protocol;
int udp_write_fd;
int udp_read_fd;
int gui_fd;
int video_mem_fd;
uint8_t* vidmem;
uint8_t cmd[10];

int set_game_variables();
void make_cross();
void make_circle();
void draw_cross(int x, int y);
void draw_figure(int x, int y, int fig);
void draw_table();
int get_vidmem();
int32_t setup_gui();
void handle_click(int id);
void game_loop();
void process_cmd(uint8_t* cmd);
int check_winner();

#endif
