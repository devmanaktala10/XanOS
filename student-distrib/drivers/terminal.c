#include "terminal.h"

/* video memory location */
static char* video_mem = (char *)VIDEO;

/* this macro gets the lower byte from the input 16bits */
#define LOWERBYTE(n) ((n)&0x00FF)

/* this macro gets to upper byte from the input 16bits */
#define UPPERBYTE(n) (((n) >> 8)&0xFF)

/* init_terminal
 *   DESCRIPTION: initializes the terminal, clears the buffers and sets the screenx,y to 0,0
 *                resets the scroll. Clears the screen.
 *   INPUTS: terminal_t terminal - the terminal struct to initialize
 *   OUTPUTS: void
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void init_terminal(terminal_t * terminal){

    /* reset the terminal variables */
    terminal->screen_x = 0;
    terminal->screen_y = 0;
    terminal->read_mode = 0;
    terminal->buffer_counter = 0;
    terminal->executed = 0;

    int i;
    /* reset the read buffer */
    for(i = 0; i < MAX_READ; i++){
        terminal->read[i] = '\0';
    }
}

/* set_current
 *   DESCRIPTION: Sets the current terminal to the input one. This function was made
 *                thinking about multiple terminal support in checkpoint 5.
 *   INPUTS: terminal_t terminal - the terminal to set as current
 *   OUTPUTS: void
 *   RETURN VALUE: None
 *   SIDE EFFECTS: the screen gets set to the terminal and changes the cursor
 */
void set_current(terminal_t * terminal){

    /* set current as input and set cursor */
    current = terminal;
    set_cursor();
}

/* write_char_current
 *   DESCRIPTION: Write a character onto the screen. Adjust the buffer values accordingly,
 *                set the cursor and handle scrolling and backspaces.
 *   INPUTS: char input - char to display
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS: the character gets printed
 */
void write_char_current(char input){

    /* make sure writing to correct swap space */
    uint32_t cached_swap = page_table_1[FOURKB_ADDRESS(VIDEO)].base_address;
    page_table_1[FOURKB_ADDRESS(VIDEO)].base_address = FOURKB_ADDRESS(VIDEO);
    set_cr((void *) page_dir);

    /* if a newline character came in */
    if(input == NEWLINE){

        /* if we are currently reading, then stop the read mode and put a \n in the buffer */
        if(current->read_mode == 1){
            current->read_mode = 0;
            current->read[current->buffer_counter++] = '\n';
        }

        /* go to next line, reset the x position, increase the y position */
        current->screen_x = 0;
        current->screen_y++;

        /* if the screen_y position goes out of the screen, we need to scroll */
        if(current->screen_y >= NUM_ROWS){
            /* scroll the screen */
            scroll_up(current, 1);
        }
    }

    /* if a backspace character came in */
    else if(input == BACKSPACE){

        /* if we are currently reading */
        if(current->read_mode == 1){

            /* if the read buffer is already empty do nothing */
            if(current->buffer_counter == 0){
                current->buffer_counter = 0;
                page_table_1[FOURKB_ADDRESS(VIDEO)].base_address = cached_swap;
                set_cr((void *) page_dir);
                return;
            }

            /* else remove the current head from the read buffer */
            else
                current->read[--(current->buffer_counter)] = '\0';
        }


        /* make the x position go back */
        current->screen_x--;

        /* if the x goes beyond the screen and we are not in the left corner */
        if(current->screen_x < 0 && current->screen_y > 0){

            /* move up */
            current->screen_y--;

            /* go the most right you can and then move backwards till you get a character */
            current->screen_x = NUM_COLS - 1;

            /* make the current last character NULL */
            *(uint8_t *)(video_mem + ((NUM_COLS * current->screen_y + current->screen_x) << 1)) = '\0';
            *(uint8_t *)(video_mem + ((NUM_COLS * current->screen_y + current->screen_x) << 1) + 1) = ATTRIB;

            /* clear the buffer character */
            while(*(uint8_t *)(video_mem + ((NUM_COLS * current->screen_y + (current->screen_x - 1)) << 1)) == '\0'){
                current->screen_x--;
                /* if there is nothing on the line, then stay at the left of that line */
                if(current->screen_x <= 0){
                    current->screen_x = 0;
                    break;
                }
            }
        }
        /* if in the top left, just stay there */
        else if(current->screen_x < 0){
            current->screen_x = 0;
        }

        /* otherwise, remove a character from the buffer and write null on the screen */
        else{
            *(uint8_t *)(video_mem + ((NUM_COLS * current->screen_y + current->screen_x) << 1)) = '\0';
            *(uint8_t *)(video_mem + ((NUM_COLS * current->screen_y + current->screen_x) << 1) + 1) = ATTRIB;
        }
    }

    /* for any other character */
    else{
        /* if we are reading */
        if(current->read_mode == 1){
            /* if we still have room to read, put character in the read buffer */
            if(current->buffer_counter < MAX_READ - 1){
                current->read[current->buffer_counter++] = input;
            }
        }

        /* put the character on the screen and the buffer */
        *(uint8_t *)(video_mem + ((NUM_COLS * current->screen_y + current->screen_x) << 1)) = input;
        *(uint8_t *)(video_mem + ((NUM_COLS * current->screen_y + current->screen_x) << 1) + 1) = ATTRIB;
        current->screen_x++;

        /* if we have gone beyond the screen, move down*/
        if(current->screen_x >= NUM_COLS){
            /* reset x to left */
            current->screen_x = 0;
            current->screen_y++;
            /* if moving down causes you to go out, then scroll */
            if(current->screen_y >= NUM_ROWS){
            /* increment the scroll counter */
            scroll_up(current, 1);
            }
        }
    }

    page_table_1[FOURKB_ADDRESS(VIDEO)].base_address = cached_swap;
    set_cr((void *) page_dir);
}

/* write_char_inactive
 *   DESCRIPTION: Write a character onto the screen. Adjust the buffer values accordingly,
 *                set the cursor and handle scrolling and backspaces.
 *   INPUTS: char input - char to display
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS: the character gets printed
 */
void write_char_inactive(char input){

    /* if a newline character came in */
    if(input == NEWLINE){

        /* go to next line, reset the x position, increase the y position */
        terminals[process_term].screen_x = 0;
        terminals[process_term].screen_y++;

        /* if the screen_y position goes out of the screen, we need to scroll */
        if(terminals[process_term].screen_y >= NUM_ROWS){
            /* scroll the screen */
            scroll_up(&terminals[process_term], 1);
        }
    }

    /* if a backspace character came in */
    else if(input == BACKSPACE){

        /* make the x position go back */
        terminals[process_term].screen_x--;

        /* if the x goes beyond the screen and we are not in the left corner */
        if(terminals[process_term].screen_x < 0 && terminals[process_term].screen_y > 0){

            /* move up */
            terminals[process_term].screen_y--;

            /* go the most right you can and then move backwards till you get a character */
            terminals[process_term].screen_x = NUM_COLS - 1;

            /* make the current last character NULL */
            *(uint8_t *)(video_mem + ((NUM_COLS * terminals[process_term].screen_y + terminals[process_term].screen_x) << 1)) = '\0';
            *(uint8_t *)(video_mem + ((NUM_COLS * terminals[process_term].screen_y + terminals[process_term].screen_x) << 1) + 1) = ATTRIB;

            /* clear the buffer character */
            while(*(uint8_t *)(video_mem + ((NUM_COLS * terminals[process_term].screen_y + (terminals[process_term].screen_x - 1)) << 1)) == '\0'){
                terminals[process_term].screen_x--;
                /* if there is nothing on the line, then stay at the left of that line */
                if(terminals[process_term].screen_x <= 0){
                    terminals[process_term].screen_x = 0;
                    break;
                }
            }
        }
        /* if in the top left, just stay there */
        else if(terminals[process_term].screen_x < 0){
            terminals[process_term].screen_x = 0;
        }

        /* otherwise, remove a character from the buffer and write null on the screen */
        else{
            *(uint8_t *)(video_mem + ((NUM_COLS * terminals[process_term].screen_y + terminals[process_term].screen_x) << 1)) = '\0';
            *(uint8_t *)(video_mem + ((NUM_COLS * terminals[process_term].screen_y + terminals[process_term].screen_x) << 1) + 1) = ATTRIB;
        }
    }

    /* for any other character */
    else{

        /* put the character on the screen and the buffer */
        *(uint8_t *)(video_mem + ((NUM_COLS * terminals[process_term].screen_y + terminals[process_term].screen_x) << 1)) = input;
        *(uint8_t *)(video_mem + ((NUM_COLS * terminals[process_term].screen_y + terminals[process_term].screen_x) << 1) + 1) = ATTRIB;
        terminals[process_term].screen_x++;

        /* if we have gone beyond the screen, move down*/
        if(terminals[process_term].screen_x >= NUM_COLS){
            /* reset x to left */
            terminals[process_term].screen_x = 0;
            terminals[process_term].screen_y++;
            /* if moving down causes you to go out, then scroll */
            if(terminals[process_term].screen_y >= NUM_ROWS){
            /* increment the scroll counter */
            scroll_up(&terminals[process_term], 1);
            }
        }
    }
}

void write_char(char input){
    
    unsigned long flags;
    cli_and_save(flags);

    if(keyint || curterm == process_term)
        write_char_current(input);
    else
        write_char_inactive(input);

    restore_flags(flags);
}

/* scroll_up
 *   DESCRIPTION: Scroll the screen up. clear the screen, write the screen buffer onto the screen
 *                adjust variables
 *   INPUTS: None
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS:  the screen scrolls up
 */
void scroll_up(terminal_t * cur, int active){

    //char screen_buf[NUM_ROWS][NUM_COLS];
    int i, j;
    // for(i = 0; i < NUM_ROWS; i++){
    //     for(j = 0; j < NUM_COLS; j++){
    //         screen_buf[i][j] = *(uint8_t *)(video_mem + ((NUM_COLS * i + j) << 1));
    //         *(uint8_t *)(video_mem + ((NUM_COLS * i + j) << 1)) = '\0'; 
    //     }
    // }

    for(i = 0; i < NUM_ROWS - 1; i++){
        for(j = 0; j < NUM_COLS; j++){
            *(uint8_t *)(video_mem + ((NUM_COLS * i + j) << 1)) = *(uint8_t *)(video_mem + ((NUM_COLS * (i+1) + j) << 1));
            *(uint8_t *)(video_mem + ((NUM_COLS * i + j) << 1) + 1) = ATTRIB;
        }
    }

    for(i = 0; i < NUM_COLS; i++){
        *(uint8_t *)(video_mem + ((NUM_COLS * (NUM_ROWS-1) + i) << 1)) = '\0';
        *(uint8_t *)(video_mem + ((NUM_COLS * (NUM_ROWS-1) + i) << 1) + 1) = ATTRIB;
    }

    cur->screen_x = 0;
    cur->screen_y = NUM_ROWS - 1;
}

void redraw_screen(int data) {

  uint32_t cached_video = page_table_1[FOURKB_ADDRESS(VIDEO)].base_address;
  page_table_1[FOURKB_ADDRESS(VIDEO)].base_address = FOURKB_ADDRESS(VIDEO);

  uint32_t phy_addr = FOURKB * (curterm + 1);
  page_table_1[1].p = 1;
  page_table_1[1].us = 0;
  page_table_1[1].base_address = FOURKB_ADDRESS(phy_addr); 
  set_cr((void *) page_dir);

  char * swap = (char * ) FOURKB;

  int i, j;
  for(i = 0; i < NUM_ROWS; i++){
      for(j = 0; j < NUM_COLS; j++){
          *(uint8_t *)(swap + ((NUM_COLS * i + j) << 1)) = *(uint8_t *)(video_mem + ((NUM_COLS * i + j) << 1));
          *(uint8_t *)(swap + ((NUM_COLS * i + j) << 1) + 1) = *(uint8_t *)(video_mem + ((NUM_COLS * i + j) << 1) + 1);
      }
  }

  phy_addr = FOURKB * (data + 1);
  page_table_1[1].base_address = FOURKB_ADDRESS(phy_addr);
  set_cr((void *) page_dir);

  swap = (char * ) FOURKB;

  for(i = 0; i < NUM_ROWS; i++){
      for(j = 0; j < NUM_COLS; j++){
          *(uint8_t *)(video_mem + ((NUM_COLS * i + j) << 1)) = *(uint8_t *)(swap + ((NUM_COLS * i + j) << 1));
          *(uint8_t *)(video_mem + ((NUM_COLS * i + j) << 1) + 1) = *(uint8_t *)(swap + ((NUM_COLS * i + j) << 1) + 1);
      }
  }

  page_table_1[FOURKB_ADDRESS(VIDEO)].base_address = cached_video;
  page_table_1[1].p = 0;
  set_cr((void *) page_dir);

}
/* set_cursor
 *   DESCRIPTION: modify the VGA registers to set the cursor to the current x,y values
 *   INPUTS: None
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS:  The cursor gets set at the current x,y values
 */
void set_cursor()
{
    // if(process_term != curterm)
    //     return;

    unsigned long flags;
    /* transform 2d x,y to 1d coordinate via pos = x + y*N_COL */
	uint16_t pos = current->screen_y * NUM_COLS + current->screen_x;
    uint8_t lower_byte = LOWERBYTE(pos);
    uint8_t upper_byte = UPPERBYTE(pos);


    cli_and_save(flags);

    /* write to the cursor command port to set the upper byte of cursor position */
	outb(LOWER_BYTE_COMMAND, CURSOR_COMMAND_PORT);
	outb(lower_byte, CURSOR_DATA_PORT);

    /* write to the cursor command port to set the lower byte of cursor position */
	outb(UPPER_BYTE_COMMAND, CURSOR_COMMAND_PORT);
	outb(upper_byte, CURSOR_DATA_PORT);

    restore_flags(flags);

}

/* read_buffer
 *   DESCRIPTION: read the input values from the terminal till a newline. Max size of read is 128.
 *   INPUTS: void * buffer - buffer to write to
 *           int32_t nbytes - number of bytes to read
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS:  The cursor gets set at the current x,y values
 */
int32_t read_buffer(void * buffer, int32_t nbytes){

    /* check for NULL buffer */
    if(buffer == NULL){
        return 0;
    }

    /* make read mode 1 */
    terminals[process_term].read_mode = 1;

    /* write char will set this to zero when new line entered */
    while(terminals[process_term].read_mode == 1);

    /* if len of buffer is less than how much has been read, then read till len of buffer */
    int limit = 0;
    if(nbytes > terminals[process_term].buffer_counter)
        limit = terminals[process_term].buffer_counter;
    else
        limit = nbytes;

    /* put read buffer into input buffer */
    int i = 0;
    for(i = 0; i < limit; i++){
        *((uint8_t *)(buffer + i)) = (uint8_t)terminals[process_term].read[i];
    }

    /* the last character should always be new line */
    *((uint8_t *)(buffer + limit - 1)) = (uint8_t)('\n');

    uint32_t main_arg_counter = set_args(buffer, limit);

    /* reset the read buffer */
    for(i = 0; i < terminals[process_term].buffer_counter; i++){
        terminals[process_term].read[i] = '\0';
    }

    /* reset current */
    terminals[process_term].buffer_counter = 0;

    return main_arg_counter;
}

uint32_t set_args(uint8_t* buf, int32_t length) {

  if (length <= 0) {
    return 0;
  }

  args[0] = '\0';

  char main_arg[MAX_FILENAME_LENGTH];

  uint32_t main_arg_counter = 0;

  unsigned i = 0;

  while (buf[i] == ' ') {
    i++;
  }

  while (i < length - 1 && buf[i] != ' ') {
    main_arg[main_arg_counter++] = buf[i];
    i++;
  }

  main_arg[main_arg_counter] = '\n';
  arg_counter = 0;
  i++;

  while (i < length - 1) {
    args[arg_counter++] = buf[i++];
  }
  args[arg_counter] = '\0';

  memcpy(buf, main_arg, main_arg_counter + 1);
  buf[main_arg_counter + 1] = '\0';

  return main_arg_counter + 1;
}

/* write_terminal
 *   DESCRIPTION: write the input buffer onto the screen
 *   INPUTS: void * buffer - buffer to write
 *           int32_t nbytes - number of bytes to write
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS: writes the buffer to the terminal
 */
int32_t write_terminal(const void * buf, int32_t nbytes){

    /* check for NULL buffer */
    if(buf == NULL){
        return 0;
    }

    int i;
    /* write the buffer character by character */
    for(i = 0; i < nbytes; i++){
        char c =  *((char *)(buf) + i);
        write_char(c);
    }
    /* at the end of everything, set the cursor */
    set_cursor();

    return nbytes;
}

/* reset_term
 *   DESCRIPTION: reset the terminal, called by the Ctr+L
 *   INPUTS: None
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS: clears the screen and the terminal
 */
void reset_term(){
  init_terminal(current);
  clear();
  set_cursor();
}

/* terminal_open
 *   DESCRIPTION: initializes the current terminal and sets the
 *                cursor via calling helper reset term function
 *   INPUTS: None
 *   OUTPUTS: None
 *   RETURN VALUE: 0 for success
 *   SIDE EFFECTS: clears the screen and the terminal
 */
int32_t terminal_open() {

    int i,j,x;
    for(x = 0; x < NUM_GRAPH_TERMINALS; x++){
        
        terminals[x].executed = 0;
        uint32_t phy_addr = FOURKB * (x + 1);

        page_table_1[1].p = 1;
        page_table_1[1].us = 0;
        page_table_1[1].base_address = FOURKB_ADDRESS(phy_addr); 
        set_cr((void *) page_dir);

        char * swap = (char * ) FOURKB;

        for(i = 0; i < NUM_ROWS; i++){
            for(j = 0; j < NUM_COLS; j++){
                *(uint8_t *)(swap + ((NUM_COLS * i + j) << 1)) = '\0';
                *(uint8_t *)(swap + ((NUM_COLS * i + j) << 1) + 1) = ATTRIB;
            }
        }

    }

    page_table_1[1].p = 0;
    set_cr((void *)page_dir);

    curterm = 0;
    set_current(&terminals[0]);
    reset_term();
    terminals[0].executed = 1;
    return 0;
}

/* terminal_close
 *   DESCRIPTION: clears the screen, return 0
 *   INPUTS: None
 *   OUTPUTS: None
 *   RETURN VALUE: 0 for success
 *   SIDE EFFECTS: clears the screen
 */
int32_t terminal_close(){
    clear();
    return 0;
}

/* write_string
 *   DESCRIPTION: writes a null terminated string to the terminal. helper funciton used by printft
 *   INPUTS: char * input - null terminated string
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS: writes string to screen
 */
void write_string(char * input){
    int i = 0;
    /* write string till null termination */
    while(input[i] != '\0'){
        write_char(input[i]);
        i++;
    }

    /* at the end of everything, set the cursor */
    set_cursor();
}

/* printft
 *   DESCRIPTION: printf adapted to work with the terminal
 *   INPUTS: int8_t * format - format string
 *   OUTPUTS: None
 *   RETURN VALUE: returns number of string left to write
 *   SIDE EFFECTS: writes string to screen
 */
int32_t printft(int8_t *format, ...) {

    /* Pointer to the format string */
    int8_t* buf = format;

    /* Stack pointer for the other parameters */
    int32_t* esp = (void *)&format;
    esp++;

    while (*buf != '\0') {
        switch (*buf) {
            case '%':
                {
                    int32_t alternate = 0;
                    buf++;

format_char_switch:
                    /* Conversion specifiers */
                    switch (*buf) {
                        /* Print a literal '%' character */
                        case '%':
                            write_char('%');
                            break;

                        /* Use alternate formatting */
                        case '#':
                            alternate = 1;
                            buf++;
                            /* Yes, I know gotos are bad.  This is the
                             * most elegant and general way to do this,
                             * IMHO. */
                            goto format_char_switch;

                        /* Print a number in hexadecimal form */
                        case 'x':
                            {
                                int8_t conv_buf[64];
                                if (alternate == 0) {
                                    itoa(*((uint32_t *)esp), conv_buf, 16);
                                    write_string(conv_buf);
                                } else {
                                    int32_t starting_index;
                                    int32_t i;
                                    itoa(*((uint32_t *)esp), &conv_buf[8], 16);
                                    i = starting_index = strlen(&conv_buf[8]);
                                    while(i < 8) {
                                        conv_buf[i] = '0';
                                        i++;
                                    }
                                    write_string(&conv_buf[starting_index]);
                                }
                                esp++;
                            }
                            break;

                        /* Print a number in unsigned int form */
                        case 'u':
                            {
                                int8_t conv_buf[36];
                                itoa(*((uint32_t *)esp), conv_buf, 10);
                                write_string(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a number in signed int form */
                        case 'd':
                            {
                                int8_t conv_buf[36];
                                int32_t value = *((int32_t *)esp);
                                if(value < 0) {
                                    conv_buf[0] = '-';
                                    itoa(-value, &conv_buf[1], 10);
                                } else {
                                    itoa(value, conv_buf, 10);
                                }
                                write_string(conv_buf);
                                esp++;
                            }
                            break;

                        /* Print a single character */
                        case 'c':
                            write_char((uint8_t) *((int32_t *)esp));
                            esp++;
                            break;

                        /* Print a NULL-terminated string */
                        case 's':
                            write_string(*((int8_t **)esp));
                            esp++;
                            break;

                        default:
                            break;
                    }

                }
                break;

            default:
                write_char(*buf);
                break;
        }
        buf++;
    }
    set_cursor();
    return (buf - format);
}

int switch_terminal(int data) {

  unsigned long flags;
  cli_and_save(flags);

  if (current == &terminals[data]) {
    restore_flags(flags);
    return 0;
  }

  redraw_screen(data);
  swap_terminal_pages(curterm, data);

  if(terminals[data].executed == 0){

    prevterm = curterm;
    curterm = data;
    set_current(&terminals[data]);
    restore_flags(flags);
    return 1;

  }

  curterm = data;
  set_current(&terminals[data]);
  restore_flags(flags);
  return 0;

}

void swap_terminal_pages(int cterm, int nterm){

    if(process_term == nterm){

        page_table_1[FOURKB_ADDRESS(VIDEO)].base_address = FOURKB_ADDRESS(VIDEO);
        set_cr((void *) page_dir);

    }
    else{

        uint32_t phy_addr = FOURKB * (nterm + 1);
        page_table_1[FOURKB_ADDRESS(VIDEO)].base_address = FOURKB_ADDRESS(phy_addr);
        set_cr((void *) page_dir);

    }

}