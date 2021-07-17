#include <stdint.h>

#include "ece391support.h"
#include "ece391syscall.h"
#include "ece391guistructs.h"
#include "ece391TicTacToe.h"

int main() {
  if (10 != set_game_variables()) {
    return -1;
  }

  if (0 != get_vidmem()) {
    return -1;
  }

  waiting = !protocol;
  disable_handler = 1;

  if (0 != setup_gui()) {
    return -1;
  }

  // make_cross();
  draw_table();
  // draw_cross(100, 100);
  game_loop();

  return 0;
}

int set_game_variables() {

  char buf[BUFSIZE];
  int32_t bytes;
  int temp_fd;

  if (-1 == (temp_fd = ece391_open((uint8_t*) "circle.txt")) ) {
    ece391_fdputs(1, (uint8_t*) "Circle.txt Not Found");
  }
  ece391_read(temp_fd, circle, 800);

  if (-1 == (temp_fd = ece391_open((uint8_t*) "cross.txt")) ) {
    ece391_fdputs(1, (uint8_t*) "Cross.txt Not Found");
  }
  ece391_read(temp_fd, cross, 800);

  ece391_fdputs(1, (uint8_t*) "Client: 0, Host: 1\n");
  if (-1 == ece391_read(0, buf, BUFSIZE-1)) {
      ece391_fdputs(1, (uint8_t*) "Can't read the number from keyboard.\n");
   return 3;
  }

  if (buf[0] == '0') {
    protocol = 0;
  } else if (buf[0] == '1') {
    protocol = 1;
  } else {
    ece391_fdputs(1, (uint8_t*) "Invalid Input\n");
    return 0;
  }

  ece391_fdputs(1, (uint8_t*) "Enter udp port to open...\n");
  bytes = ece391_read(0, buf, BUFSIZE-1);
  if (-1 == bytes) {
      ece391_fdputs(1, (uint8_t*) "Can't read the udp path.\n");
   return 3;
  }

  if (buf[bytes - 1] == '\n') {
    buf[bytes - 1] = '\0';
  }

  if (-1 == (udp_write_fd = ece391_open((uint8_t*) buf))) {
      ece391_fdputs (1, (uint8_t*)"Could not open port\n");
      return 2;
  }

  ece391_fdputs(1, (uint8_t*)"Change Sender Port? 1 for Yes, or press any other key for No...\n");
  if (-1 == ece391_read(0, buf, BUFSIZE-1)) {
      ece391_fdputs(1, (uint8_t*) "Can't read the number from keyboard.\n");
   return 3;
  }

  if (buf[0] == '1') {
    char udp_sender_port[6];
    ece391_fdputs(1, (uint8_t*) "Enter udp sender port...\n");
    bytes = ece391_read(0, udp_sender_port, 5);
    if (bytes == -1) {
        ece391_fdputs(1, (uint8_t*) "Can't read the udp path.\n");
     return 3;
    }

    udp_sender_port[bytes] = '\0';
    udp_sender_port[bytes - 1] = '\0';

    if (-1 == ioctl(udp_write_fd, 0x1, (unsigned long) udp_sender_port)) {
      ece391_fdputs(1, (uint8_t*) "Could not change sender port\n");
      return 0;
    } else {
      ece391_fdputs(1, (uint8_t*) "Sender Port Changed\n");
    }
  }

  ece391_fdputs(1, (uint8_t*) "Enter udp port to read from...\n");
  if (-1 == (bytes = ece391_read(0, buf, BUFSIZE-1))) {
      ece391_fdputs(1, (uint8_t*) "Can't read the udp path.\n");
   return 3;
  }

  if (buf[bytes - 1] == '\n') {
    buf[bytes - 1] = '\0';
  }

  if (-1 == (udp_read_fd = ece391_open((uint8_t*) buf))) {
      ece391_fdputs (1, (uint8_t*)"Could not open port\n");
      return 2;
  }


  return 10;
}

int get_vidmem() {
  if (-1 == ece391_vidmap((uint8_t**) &vidmem)) {
    ece391_fdputs(1, (uint8_t*) "Unable to Get Video memory\n");
    return -1;
  }

  return 0;
}

void draw_table() {
  uint32_t y_byte_height = Y_HEIGHT / 8;
  uint32_t x_byte_width = X_WIDTH / 8;

  uint32_t start_y = 4;
  uint32_t box_size = (y_byte_height - 2 * start_y) / 3;
  uint32_t start_x = (x_byte_width - 3 * box_size) / 2;

  unsigned i, j;
  // draw 2 vertical parallel Lines
  for (i = start_y * 8; i < (y_byte_height - start_y) * 8; i++) {
    *(vidmem + (i * x_byte_width) + box_size + start_x) = 0xFF;
    *(vidmem + (i * x_byte_width) + (box_size * 2) + start_x) = 0xFF;
  }

  // draw 2 horizontal parallel Lines
  for (i = 0; i < 8 ; i++) {
    for (j = start_x; j < x_byte_width - start_x; j++) {
      *(vidmem + ( (box_size + start_y) * 8 - 4 + i) * x_byte_width + j) = 0xFF;
      *(vidmem + ( (2 * box_size + start_y) * 8 - 4 + i) * x_byte_width + j) = 0xFF;
    }
  }

}

void make_cross() {
  // uint32_t y_byte_height = Y_HEIGHT / 8;
  // uint32_t x_byte_width = X_WIDTH / 8;
  //
  // int box_size = 14;
  // uint16_t rmap = 0xFF00;
  // uint16_t lmap = 0x00FF;
  // int offset = 0;
  // uint32_t start_x = x;
  // uint32_t start_y = y;
  //
  // unsigned i, j;
  // for (i = 0; i < 14; i++) {
  //   if (i == 2) {
  //     i = 12;
  //   }
  //
  //   for (j = 0; j < 14 * 8; j++) {
  //     cross[j][i] = 0x00;
  //   }
  // }
  //
  // for (j = 0; j < 14 * 8; j++) {
  //   if (j == 2 * 8) {
  //     j = 11 * 8;
  //   }
  //   for (i = 0; i < 14; i++) {
  //     cross[j][i] = 0x00;
  //   }
  // }
  //
  // unsigned offset = 2;
  // for (j = 2 * 8; j < 11 * 8; j++) {
  //
  //   cross[j][offset] = rmap >> 8;
  //   cross[j][offset + 1] = (rmap & 0x00FF);
  //
  //   rmap = rmap >> 1;
  //
  //   if ((rmap & 0x00FF) == 0x00FF) {
  //     offset++;
  //     rmap = 0xFF00;
  //   }
  //
  //   cross[j][14 - offset] = (lmap & 0x00FF);
  //   cross[j][14 - offset - 1] = (((lmap << 1) & 0xFF00) >> 8);
  //
  //   lmap = lmap << 1;
  //
  //   if (lmap == 0xFF00) {
  //     lmap = 0x00FF;
  //   }
  //
  //   if (offset == 12) {
  //     break;
  //   }
  //
  // }

}

void draw_cross(int start_x, int start_y) {
  // unsigned i,j;
  // unsigned cross_i = 0;
  // unsigned cross_j = 0;
  //
  // for (i = start_x / 8; i < start_x / 8 + 14; i++) {
  //   for (j = start_y; j < start_y + 14 * 8; j++) {gui_fd = ece391_open((uint8_t*)"gui");
  //
  // ioctl(gui_fd, GUI_IOCTL_CLEAR_SCREEN, 0);
  // ioctl(gui_fd, GUI_IOCTL_DISABLE_SCREEN, 0);
  //     *(vidmem + j * (X_WIDTH / 8) + i) = cross[cross_j++][cross_i];
  //   }
  //   cross_i++;
  // }
}

int32_t setup_gui() {
  gui_fd = ece391_open((uint8_t*)"gui");

  ioctl(gui_fd, GUI_IOCTL_CLEAR_SCREEN, 0);
  ioctl(gui_fd, GUI_IOCTL_DISABLE_SCREEN, 0);

  gui_user_element_t box;
  gui_handler_addition_t handler;
  int32_t ret_val;

  // button 1
  box.x = 21 * 8;
  box.y = 6 * 8;
  box.width = 10 * 8;
  box.height = 10 * 8;
  box.flags = DEFAULT_BOX_TRANSPARENT;
  box_id[0] = ioctl(gui_fd, GUI_IOCTL_ADD_ELEMENT, (unsigned long)(&box));

  handler.handler = (void *)handle_click;
  handler.id = box_id[0];
  ret_val = ioctl(gui_fd, GUI_IOCTL_ADD_HANDLER, (unsigned long)(&handler));

  if(ret_val == -1) {
      ece391_fdputs(1, (uint8_t *)"Handler Addition Failed");
      return 1;
  }

  // button 2
  box.x = (21 + 14) * 8;
  box.y = 6 * 8;
  box.width = 10 * 8;
  box.height = 10 * 8;
  box.flags = DEFAULT_BOX_TRANSPARENT;
  box_id[1] = ioctl(gui_fd, GUI_IOCTL_ADD_ELEMENT, (unsigned long)(&box));

  handler.handler = (void *)handle_click;
  handler.id = box_id[1];
  ret_val = ioctl(gui_fd, GUI_IOCTL_ADD_HANDLER, (unsigned long)(&handler));

  if(ret_val == -1) {
      ece391_fdputs(1, (uint8_t *)"Handler Addition Failed");
      return 1;
  }

  // button 3
  box.x = (21 + 2 * 14) * 8;
  box.y = 6 * 8;
  box.width = 10 * 8;
  box.height = 10 * 8;
  box.flags = DEFAULT_BOX_TRANSPARENT;
  box_id[2] = ioctl(gui_fd, GUI_IOCTL_ADD_ELEMENT, (unsigned long)(&box));

  handler.handler = (void *)handle_click;
  handler.id = box_id[2];
  ret_val = ioctl(gui_fd, GUI_IOCTL_ADD_HANDLER, (unsigned long)(&handler));

  if(ret_val == -1) {
      ece391_fdputs(1, (uint8_t *)"Handler Addition Failed");
      return 1;
  }

  // button 4
  box.x = 21 * 8;
  box.y = (6 + 14) * 8;
  box.width = 10 * 8;
  box.height = 10 * 8;
  box.flags = DEFAULT_BOX_TRANSPARENT;
  box_id[3] = ioctl(gui_fd, GUI_IOCTL_ADD_ELEMENT, (unsigned long)(&box));

  handler.handler = (void *)handle_click;
  handler.id = box_id[3];
  ret_val = ioctl(gui_fd, GUI_IOCTL_ADD_HANDLER, (unsigned long)(&handler));

  if(ret_val == -1) {
      ece391_fdputs(1, (uint8_t *)"Handler Addition Failed");
      return 1;
  }

  // button 5
  box.x = (21 + 14) * 8;
  box.y = (6 + 14) * 8;
  box.width = 10 * 8;
  box.height = 10 * 8;
  box.flags = DEFAULT_BOX_TRANSPARENT;
  box_id[4] = ioctl(gui_fd, GUI_IOCTL_ADD_ELEMENT, (unsigned long)(&box));

  handler.handler = (void *)handle_click;
  handler.id = box_id[4];
  ret_val = ioctl(gui_fd, GUI_IOCTL_ADD_HANDLER, (unsigned long)(&handler));

  if(ret_val == -1) {
      ece391_fdputs(1, (uint8_t *)"Handler Addition Failed");
      return 1;
  }

  // button 6
  box.x = (21 + 2 * 14) * 8;
  box.y = (6 + 14) * 8;
  box.width = 10 * 8;
  box.height = 10 * 8;
  box.flags = DEFAULT_BOX_TRANSPARENT;
  box_id[5] = ioctl(gui_fd, GUI_IOCTL_ADD_ELEMENT, (unsigned long)(&box));

  handler.handler = (void *)handle_click;
  handler.id = box_id[5];
  ret_val = ioctl(gui_fd, GUI_IOCTL_ADD_HANDLER, (unsigned long)(&handler));

  if(ret_val == -1) {
      ece391_fdputs(1, (uint8_t *)"Handler Addition Failed");
      return 1;
  }

  // button 7
  box.x = (21) * 8;
  box.y = (6 + 2 * 14) * 8;
  box.width = 10 * 8;
  box.height = 10 * 8;
  box.flags = DEFAULT_BOX_TRANSPARENT;
  box_id[6] = ioctl(gui_fd, GUI_IOCTL_ADD_ELEMENT, (unsigned long)(&box));

  handler.handler = (void *)handle_click;
  handler.id = box_id[6];
  ret_val = ioctl(gui_fd, GUI_IOCTL_ADD_HANDLER, (unsigned long)(&handler));

  if(ret_val == -1) {
      ece391_fdputs(1, (uint8_t *)"Handler Addition Failed");
      return 1;
  }

  // button 8
  box.x = (21 + 14) * 8;
  box.y = (6 + 2 * 14) * 8;
  box.width = 10 * 8;
  box.height = 10 * 8;
  box.flags = DEFAULT_BOX_TRANSPARENT;
  box_id[7] = ioctl(gui_fd, GUI_IOCTL_ADD_ELEMENT, (unsigned long)(&box));

  handler.handler = (void *)handle_click;
  handler.id = box_id[7];
  ret_val = ioctl(gui_fd, GUI_IOCTL_ADD_HANDLER, (unsigned long)(&handler));

  if(ret_val == -1) {
      ece391_fdputs(1, (uint8_t *)"Handler Addition Failed");
      return 1;
  }

  // button 9
  box.x = (21 + 2 * 14) * 8;
  box.y = (6 + 2 * 14) * 8;
  box.width = 10 * 8;
  box.height = 10 * 8;
  box.flags = DEFAULT_BOX_TRANSPARENT;
  box_id[8] = ioctl(gui_fd, GUI_IOCTL_ADD_ELEMENT, (unsigned long)(&box));

  handler.handler = (void *)handle_click;
  handler.id = box_id[8];
  ret_val = ioctl(gui_fd, GUI_IOCTL_ADD_HANDLER, (unsigned long)(&handler));

  if(ret_val == -1) {
      ece391_fdputs(1, (uint8_t *)"Handler Addition Failed");
      return 1;
  }

  return 0;

}

void handle_click(int id) {
  if (waiting) {
    return;
  }

  if (disable_handler) {
    return;
  }

  int x, y;

  if (id == box_id[0]) {

    if (game_arr[0] != -1) {
      return;
    }
    game_arr[0] = protocol;
    ece391_write(udp_write_fd, (void*) "0", 1);
    x = 21 * 8;
    y = 6 * 8;
    draw_figure(x, y, protocol);
  } else if (id == box_id[1]) {

    if (game_arr[1] != -1) {
      return;
    }
    game_arr[1] = protocol;
    ece391_write(udp_write_fd, (void*) "1", 1);

    x = (21 + 14) * 8;
    y = 6 * 8;
    draw_figure(x, y, protocol);
  } else if (id == box_id[2]) {

    if (game_arr[2] != -1) {
      return;
    }
    game_arr[2] = protocol;
    ece391_write(udp_write_fd, (void*) "2", 1);

    x = (21 + 2 * 14) * 8;
    y = 6 * 8;
    draw_figure(x, y, protocol);
  } else if (id == box_id[3]) {

    if (game_arr[3] != -1) {
      return;
    }
    game_arr[3] = protocol;
    ece391_write(udp_write_fd, (void*) "3", 1);

    x = 21 * 8;
    y = (6 + 14) * 8;
    draw_figure(x, y, protocol);
  } else if (id == box_id[4]) {

    if (game_arr[4] != -1) {
      return;
    }
    game_arr[4] = protocol;
    ece391_write(udp_write_fd, (void*) "4", 1);

    x = (21 + 14) * 8;
    y = (6 + 14) * 8;
    draw_figure(x, y, protocol);
  } else if (id == box_id[5]) {

    if (game_arr[5] != -1) {
      return;
    }
    game_arr[5] = protocol;
    ece391_write(udp_write_fd, (void*) "5", 1);

    x = (21 + 2 * 14) * 8;
    y = (6 + 14) * 8;
    draw_figure(x, y, protocol);
  } else if (id == box_id[6]) {

    if (game_arr[6] != -1) {
      return;
    }
    game_arr[6] = protocol;
    ece391_write(udp_write_fd, (void*) "6", 1);

    x = (21) * 8;
    y = (6 + 2 * 14) * 8;
    draw_figure(x, y, protocol);
  } else if (id == box_id[7]) {

    if (game_arr[7] != -1) {
      return;
    }
    game_arr[7] = protocol;
    ece391_write(udp_write_fd, (void*) "7", 1);

    x = (21 + 14) * 8;
    y = (6 + 2 * 14) * 8;
    draw_figure(x, y, protocol);
  } else if (id == box_id[8]) {

    if (game_arr[8] != -1) {
      return;
    }
    game_arr[8] = protocol;
    ece391_write(udp_write_fd, (void*) "8", 1);

    x = (21 + 2 * 14) * 8;
    y = (6 + 2 * 14) * 8;
    draw_figure(x, y, protocol);
  }

  int32_t retval = check_winner();
  if (retval == 0) {
    ece391_fdputs(1, (uint8_t*) "KNOT WINS!\n");
    ece391_halt(0);
  } else if (retval == 1) {
    ece391_fdputs(1, (uint8_t*) "CROSS WINS!\n");
    ece391_halt(0);
  }

  waiting = 1;

  // ece391_fdputs(1, (uint8_t*) "Here\n");
}

void draw_figure(int x, int y, int fig) {
  uint8_t* buf;
  uint32_t x_byte_width = X_WIDTH / 8;

  if (fig == 1) {
    buf = cross;
  } else if (fig == 0) {
    buf = circle;
  } else {
    return;
  }

  uint32_t start_x = x / 8;

  unsigned i, j;
  for (i = 0; i < 10; i++) {
    for (j = 0; j < 80; j++) {
      *(vidmem + (y + j + 16) * x_byte_width + start_x + i) = buf[i + j * 10];
    }
  }
}

void game_loop() {
  int bytes, retval;

  if (protocol) {
    ece391_fdputs(1, (uint8_t*) "You are CROSS, You Start\n");
    waiting = 0;
  } else {
    ece391_fdputs(1, (uint8_t*) "You are KNOTS\n");
    waiting = 1;
  }

  while(1) {

    if (waiting) {
      disable_handler = 1;
      ece391_fdputs(1, (uint8_t*) "Waiting for Opponent Move\n");
      cmd[0] = '\0';
      bytes = ece391_read(udp_read_fd, cmd, 0);
      process_cmd(cmd);

      retval = check_winner();
      if (retval == 0) {
        ece391_fdputs(1, (uint8_t*) "KNOT WINS!\n");
        ece391_halt(0);
        break;
      } else if (retval == 1) {
        ece391_fdputs(1, (uint8_t*) "CROSS WINS!\n");
        ece391_halt(0);
        break;
      }
      waiting = 0;

      ece391_fdputs(1, (uint8_t*) "Your Move\n");
    } else {
      disable_handler = 0;
      while (1) {
        if ( (volatile int) waiting ) {
          break;
        }
      }
    }

  }
}

void process_cmd(uint8_t* cmd) {

  int x, y;

  switch (cmd[0]) {
    case '0':
      game_arr[0] = !protocol;
      x = 21 * 8;
      y = 6 * 8;
      draw_figure(x, y, !protocol);
      break;

    case '1':
      game_arr[1] = !protocol;
      x = (21 + 14) * 8;
      y = 6 * 8;
      draw_figure(x, y, !protocol);
      break;

    case '2':
      game_arr[2] = !protocol;
      x = (21 + 2 * 14) * 8;
      y = 6 * 8;
      draw_figure(x, y, !protocol);
      break;

    case '3':
      game_arr[3] = !protocol;
      x = 21 * 8;
      y = (6 + 14) * 8;
      draw_figure(x, y, !protocol);
      break;

    case '4':
      game_arr[4] = !protocol;
      x = (21 + 14) * 8;
      y = (6 + 14) * 8;
      draw_figure(x, y, !protocol);
      break;

    case '5':
      game_arr[5] = !protocol;
      x = (21 + 2 * 14) * 8;
      y = (6 + 14) * 8;
      draw_figure(x, y, !protocol);
      break;

    case '6':
      game_arr[6] = !protocol;
      x = (21) * 8;
      y = (6 + 2 * 14) * 8;
      draw_figure(x, y, !protocol);
      break;

    case '7':
      game_arr[7] = !protocol;
      x = (21 + 14) * 8;
      y = (6 + 2 * 14) * 8;
      draw_figure(x, y, !protocol);
      break;

    case '8':
      game_arr[8] = !protocol;
      x = (21 + 2 * 14) * 8;
      y = (6 + 2 * 14) * 8;
      draw_figure(x, y, !protocol);
      break;
  }

  ece391_fdputs(1, (uint8_t*) "PROCESS_CMD\n");
}

int check_winner() {

    if((game_arr[0] == game_arr[1]) && (game_arr[1] == game_arr[2]) && game_arr[0] != -1) return game_arr[0];
    else if ((game_arr[3] == game_arr[4]) && (game_arr[4] == game_arr[5]) && game_arr[4] != -1) return game_arr[3];
    else if ((game_arr[6] == game_arr[7]) && (game_arr[7] == game_arr[8]) && game_arr[6] != -1) return game_arr[6];
    else if ((game_arr[0] == game_arr[3]) && (game_arr[3] == game_arr[6]) && game_arr[0] != -1) return game_arr[0];
    else if ((game_arr[1] == game_arr[4]) && (game_arr[4] == game_arr[7]) && game_arr[1] != -1) return game_arr[1];
    else if ((game_arr[2] == game_arr[5]) && (game_arr[5] == game_arr[8]) && game_arr[2] != -1) return game_arr[2];
    else if ((game_arr[0] == game_arr[4]) && (game_arr[4] == game_arr[8]) && game_arr[0] != -1) return game_arr[0];
    else if ((game_arr[2] == game_arr[4]) && (game_arr[4] == game_arr[6]) && game_arr[2] != -1) return game_arr[2];
    else return -1;

  }
