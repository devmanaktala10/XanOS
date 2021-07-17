#include "keyboard.h"

#define LEAVE_INDEX 0x81
#define MAP_LEN 0x3B
/* 0 - ESC
 * 1 - BACKSPACE
 * 2 - TAB
 * 3 - ENTER
 * 4 - LEFT CONTROL
 * 5 - LEFT SHIFT
 * 6 - RIGHT SHIFT
 * 7 - ALT
 * 8 - CAPS*/
volatile int l_shift = 0;
volatile int r_shift = 0;
volatile int ctrl = 0;
volatile int caps = 0;
volatile int alt = 0;
volatile int click_on_elem = 0;

const unsigned char map[MAP_LEN] = {'\0', 0, '1', '2', '3', '4', '5', '6', '7'
    , '8', '9', '0', '-', '=', 1, 2, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o'
    , 'p', '[', ']', '\n', 4, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';'
    , '\'', '`', 5, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 6, '*'
    , 7, ' ', 8};

const unsigned char shift_map[MAP_LEN] = {'\0', 0, '!', '@', '#', '$', '%', '^'
    , '&', '*', '(', ')', '_', '+', 1, 2, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I'
    , 'O', 'P', '{', '}', '\n', 4, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':'
    , '"', '~', 5, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 6, '*'
    , 7, ' ', 8};

/* setup_ps2_controller
 * DESCRIPTION: Initialization routine for PS/2 controller to enable the keyboard
 *              interrupts.
 * INPUTS: None
 * RETURN VALUE: None
 * OUTPUTS: Prints error message if failure
 * SIDE EFFECTS: PS/2 keyboard initialized and ready to generate interrupts */
void setup_ps2_controller() {

  unsigned long flags;
  uint8_t config_byte, response;

  cli_and_save(flags);

  printf("ENABLING PS2 Controller\n");

  // Start by disabling ps2 devices so they don't mess up initialization
  outb(DISABLE_PORT_1, PS2_CONT);
  outb(DISABLE_PORT_2, PS2_CONT);

  // Flush the ouput buffer of controller to remove any old data
  inb(PS2_CONT_DATA);

  // Read old value of config byte from PS2
  outb(READ_CONFIG_CMD, PS2_CONT);

  // While loop waits until PIC has Processed Commands, and ready to send reply
  // This is determined by checking if Bit 1 of status register is 1
  while ((inb(PS2_CONT) & 1) == 0);
  config_byte = inb(PS2_CONT_DATA);

  // We clean bits 1, 2, & 6 of the config byte
  // Bit 1, 2 disable interrupts from PORT 1 & 2
  // Bit 6 needs to be cleared for initialization
  config_byte = (config_byte & PS2_CONFIG_CLEAN);

  // Writes the new config byte to controller
  outb(WRITE_CONFIG_CMD, PS2_CONT);
  // Wait until output buffer is clear by checking bit 2 of status register
  while ((inb(PS2_CONT) & 2) != 0);
  outb(config_byte, PS2_CONT_DATA);

  outb(SELF_TEST_PS2, PS2_CONT);
  while ((inb(PS2_CONT) & 1) == 0);  // again PIC has Processed Commands
  response = inb(PS2_CONT_DATA);

  if (response != PS2_TEST_PASSED) {
    printf("CONTROLLER SELF TEST FAILED\n");
  }

  // outb(READ_CONFIG_CMD, PS2_CONT);
  // while ((inb(PS2_CONT) & 1) == 0);
  // config_byte = inb(PS2_CONT_DATA);
  // if((config_byte& 0x10>>4)){
  //   printf("no port 2");
  // }

  // Writes the new config byte to controller again because PIC might get reset
  // by SELF_TEST_PS2
  outb(WRITE_CONFIG_CMD, PS2_CONT);
  while ((inb(PS2_CONT) & 2) != 0);
  outb(config_byte, PS2_CONT_DATA);


  outb(PORT1_TEST, PS2_CONT);
  while ((inb(PS2_CONT) & 1) == 0);  // PIC has Processed Commands
  response = inb(PS2_CONT_DATA);

  if (response != PORT1_TEST_PASSED) {
    printf("PORT 1 TEST FAILED\n");
  }

  //test the second port (mouse)
  outb(PORT2_TEST, PS2_CONT);
  while((inb(PS2_CONT) & 1) == 0);
  response = inb(PS2_CONT_DATA);

  if(response != PORT1_TEST_PASSED)
    printf("PORT 2 TEST FAILED\n");

  // Enable First port and second of PS2

  // enable interrupts on irq1 and irq2 by making bit 0,1 of config byte 1
  config_byte = config_byte | 1;
  outb(WRITE_CONFIG_CMD, PS2_CONT);
  while ((inb(PS2_CONT) & 2) != 0);
  outb(config_byte, PS2_CONT_DATA);

  outb(ENABLE_PORT_1, PS2_CONT);

  // Reset Port 1 keyboard
  while (1) {
    response = send_cmd_keyboard(RESET_KEYBOARD, 1);
    if (response == KRESEND_CMD) { // resends cmd
      continue;
    // check if reset failed
    } else if (response == KRESET_FAILED_1 || response == KRESET_FAILED_2) {
      printf("KEYBOARD RESET FAILED \n");
      break;
    } else if (response == KTEST_PASSED) {
      printf("RESET DONE\n");
      break;
    // If ACK comes get another response and check if test passed
  } else if (response == KEYBOARD_ACK) {
      while ((inb(PS2_CONT) & 1) == 0);
      response = inb(PS2_CONT_DATA);
      if (response == KTEST_PASSED) {
        printf("RESET DONE\n");
      } else {
        printf("UNKNOWN BEHAVIOR: %u\n", response);
      }
      break;
    } else {
      printf("UNKNOWN BEHAVIOR: %u\n", response);
      break;
    }
  }

  // Enable Scanning
  while (1) {
    response = send_cmd_keyboard(ENABLE_SCANNING, 1);
    // Resends Command
    if (response == KRESEND_CMD) {
      continue;
    // If ACK comes, reset is done
  } else if (response == KEYBOARD_ACK) {
      printf("KEYBOARD SCANNING ON\n");
      break;
    } else {
      printf("UNKNOWN BEHAVIOR: %u\n", response);
      break;
    }
  }

  //reset mouse
  while ((inb(PS2_CONT) & 2) != 0);
  outb(ENABLE_PORT_2,PS2_CONT);
  while ((inb(PS2_CONT) & 2) != 0);

  outb(READ_CONFIG_CMD, PS2_CONT);
  while ((inb(PS2_CONT) & 1) == 0);
  config_byte = inb(PS2_CONT_DATA);
  config_byte = config_byte | 2;
  while ((inb(PS2_CONT) & 2) != 0);
  outb(WRITE_CONFIG_CMD, PS2_CONT);

  while ((inb(PS2_CONT) & 2) != 0);
  outb(config_byte, PS2_CONT_DATA);
  //Reset Mouse
  do{
    while ((inb(PS2_CONT) & 2) != 0);
    outb(SEND_TO_MOUSE, PS2_CONT);
    response = send_cmd_keyboard(RESET_KEYBOARD, 1);
  } while(response != KEYBOARD_ACK);

   restore_flags(flags);
}

/* send_cmd_keyboard
 * DESCRIPTION: Sends Keyboard commands to the PS2 controller.
 *              First cleans input buffer, and then sends the command.
 *              If a response is expected, gets it, and returns it.
 * INPUT: cmd -- cmd to send to PS2
 *        response -- checks to see if a response is expected
 * RETURN VALUE: If response is expected, returns the response
 * OUTPUTS: If cleaning input buffer takes too long, prints timeout to screen
 * SIDE EFFECTS: May change state of PS2, and Keyboard */
uint8_t send_cmd_keyboard(uint8_t cmd, unsigned response) {
  unsigned i = 0;

  while ((inb(PS2_CONT) & 2) != 0) { // wait till input buffer is clear
    i++;
    if (i == K_CMD_TIMEOUT) {
      printf("KEYBOARD CMD TIMED OUT \n");
      return -1;
    }
  }

  outb(cmd, PS2_CONT_DATA); // send command to PS2

  // if response is expected, get it, and return it
  if (response) {
    while ((inb(PS2_CONT) & 1) == 0);
    return inb(PS2_CONT_DATA);
  } else {
    return 0;
  }
}

/* keyboard_interrupt_handler
 * DESCRIPTION: Keyboard interrupt handler. reads the scancode from the keyboard register,
 *              finds the character typed from the map lookup table and prints the character
 *              to the screen.
 * INPUT:None
 * RETURN VALUE: None
 * OUTPUTS: Prints character typed on screen
 * SIDE EFFECTS: Screen changes */
int keyboard_interrupt_handler() {

  int exec = 0;
  int sigint = 0;
  prevterm = 0;
  unsigned long flags;
  cli_and_save(flags);

  /* this marks that the character is being written by a keyboard interrupt */
  keyint = 1;
  /* Disable further keyboard interrupts */
  disable_irq(KEYBOARD_IRQ);

  /* Send EOI signal */
  send_eoi(KEYBOARD_IRQ);

  unsigned char data;
  /* read the typed scancode */
  data = inb(PS2_CONT_DATA);

  /* if key pressed */
  int flag = 0; /*set flag to 0. The flag is set to 1 if the current typed char is
               is a a shift, tab, alt, esc, caps button*/
  //if the data is a typable character
  if(data < MAP_LEN){
      switch(map[data]){

        //If CTRL in pressed
        case 4: ctrl=1;
                flag=1;
                break;

        //if Left Shift is pressed
        case 5: l_shift=1;
                flag=1;
                break;

        //if Right Shift is pressed
        case 6: r_shift=1;
                flag=1;
                break;

        //If caps lock is pressed
        case 8: caps=!caps;
                flag=1;
                break;

        // If alt is pressed
        case 7: alt = 1;
                flag = 1;
                break;

        //If ESC is pressed
        case 0: flag=1;
                break;

        //If tab is pressed
        case 2: flag=1;
                char tab[] = "   ";
                write_graphic_terminal(tab, 3);
                break;
      }

      //If none of the buttons above were pressed, we write a character to the screen.
      if(!flag){
        //if shift was pressed
        if(l_shift||r_shift){
            /*if caps lock is on and the character typed was a letter, we write a
            a lower case letter*/
            if(caps && map[data]>='a' && map[data]<='z')
              write_char_graphic_helper(map[data]);
            else write_char_graphic_helper(shift_map[data]);

        }
        // If caps lock was on and the typed letter was a letter
        else if(caps && map[data]>='a' && map[data]<='z'){
            //if ctrl-l was pressed clear the terminal
            if(ctrl && map[data]=='l') reset_graphic_term(curterm);
            else if(ctrl && map[data]=='c') {
            sigint = 1;
            }
            //else write the letter in caps
            else write_char_graphic_helper(map[data] - 32); //printft("%c", (map[data]-32));
          }
        // if ctrl-l is pressed clear the screen
        else if(ctrl && map[data]=='l') {
            reset_graphic_term(curterm);
        }
        else if(ctrl && map[data]=='c') {
          sigint = 1;
        }

        //else simply write the character
        else write_char_graphic_helper(map[data]);//printft("%c", map[data]);
      }
  } else if (data >= F1 && data <= F10) {
      if (alt) {
        exec = switch_graphic_terminal(data - F1);
      }
  }
  // else we check if any button was released
  else {
    switch(data){
        //left shift was released
        case L_SHIFT_REL: l_shift = 0;
                          break;
        //right shift was released
        case R_SHIFT_REL: r_shift = 0;
                          break;
        //ctrl was released
        case CTRL_REL:    ctrl = 0;
                          break;
        // alt was release
        case ALT_REL:     alt = 0;
                          break;
    }
  }
  
  /* re-enable key_board interrupts */
  enable_irq(KEYBOARD_IRQ);

  keyint = 0;

  if(exec){

    asm volatile("                         \n\
                movl %%esp, 0(%%eax)      \n\
                movl %%ebp, 4(%%eax)      \n\
                "
                :
                :"a"(&sched_swaps[process_term])
                :"memory"
    );
    set_terminal_bitmap((data - F1), 1);
    execute_new_term((uint8_t *) "/utils/shell", data - F1);
  }

  if (sigint) {
    sig_desc(2);
    // cli();
    // context_switch(schedule[curterm]);
    // return 1;
  }

  restore_flags(flags);

  return 0;
}

void init_mouse(){
  unsigned long flags;
  cli_and_save(flags);
  while(1){
  while ((inb(PS2_CONT) & 2) != 0);
  outb(SEND_TO_MOUSE,PS2_CONT);
  uint8_t response;
  response = send_cmd_keyboard(MOUSE_ENABLE_DATA, 1);
  if(response == KRESEND_CMD)
     continue;

  else if (response == KEYBOARD_ACK) {
      printf("MOUSEON\n");
      break;
  }
  else {
      printf("OH NO BEHAVIOR: %u\n", response);
      break;
  }

}
   pos_x = 1280/2;
   pos_y = 1024/2;
  restore_flags(flags);
}

int mouse_interrupt_handler(){
  unsigned long flags;
  cli_and_save(flags);

  icon_exec = 0;

  disable_irq(MOUSE_IRQ);
  send_eoi(MOUSE_IRQ);

  uint8_t byte0 = inb(PS2_CONT_DATA);
  uint8_t byte1 = inb(PS2_CONT_DATA);
  uint8_t byte2 = inb(PS2_CONT_DATA);

  if(byte0 & 0x06) {

    enable_irq(MOUSE_IRQ);
    restore_flags(flags);
    return -1;

  }

  click_left = (byte0) & 0x01;

  if(click_left && click_on_elem != 1){

    gui_element_t* t = check_click_on_elem(pos_x + 7, pos_y);

  if(t != NULL){

    mouse_args_t mouse_args;
    mouse_args.byte0 = byte0;
    mouse_args.byte1 = byte1;
    mouse_args.byte2 = byte2;

    click_handler_helper(t, &mouse_args);
      
  }

  else 
    click_on_elem = 1;

  }
  if(!click_left) 
    click_on_elem =0;

  pos_x += byte1 - ((byte0 << 4) & 0x100);
  pos_y -= byte2 - ((byte0 << 3) & 0x100);

  if(pos_x < 0) pos_x = 0;
  if(pos_x > 1280 - 32) pos_x = 1280 - 32;
  if(pos_y < 0) pos_y = 0;
  if(pos_y > 1024 - 32) pos_y = 1024 - 32;

  if(icon_exec == 1){

    int term = find_free_terminal();
    curterm = term;

    enable_irq(MOUSE_IRQ);
  
    if(term != -1){
        asm volatile("                      \n\
                  movl %%esp, 0(%%eax)      \n\
                  movl %%ebp, 4(%%eax)      \n\
                  "
                  :
                  :"a"(&sched_swaps[process_term])
                  :"memory"
      );
      
      execute_new_term(exec_path, term);
    }
  }

  enable_irq(MOUSE_IRQ);
  restore_flags(flags);
  return 0;
  
}
