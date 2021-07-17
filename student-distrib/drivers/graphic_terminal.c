#include "graphic_terminal.h"

static const uint32_t vidloc = 0xFC000000;

/* terminal_open
 *   DESCRIPTION: initializes the current terminal and sets the
 *                cursor via calling helper reset term function
 *   INPUTS: None
 *   OUTPUTS: None
 *   RETURN VALUE: 0 for success
 *   SIDE EFFECTS: clears the screen and the terminal
 */
int32_t graphic_terminal_open() {

    int x;
    for(x = 0; x < NUM_GRAPH_TERMINALS; x++){

        gterminals[x].screen_x = 0;
        gterminals[x].screen_y = 0;
        gterminals[x].read_mode = 0;
        gterminals[x].buffer_counter = 0;
        gterminals[x].executed = 0;
        gterminals[x].disabled = 0;
        gterminals[x].background = COLOR_EXPAND_WHITE;
        gterminals[x].text = COLOR_EXPAND_BLACK;
        gterminals[x].r1_mode = 0;
        memset(gterminals[x].read, 0x00, 128);
        memset((void *)(vidloc + TERMINAL_0_START_OFFSET + x * TERMINAL_SIZE), 0x00, 32*1024);

    }

    terminal_bitmap = 0;
    curterm = 0;
    return 0;
}

void write_char_graphic_helper(char input){
    
    unsigned long flags;
    cli_and_save(flags);

    if(curterm == -1) return;

    if(keyint){
        if(gterminals[curterm].disabled != 1){
            write_char_graphic(input, curterm);
        }
    }
    else{
        if(curterm == process_term){
            write_char_graphic(input, curterm);
        }
        else{
            write_char_graphic(input, process_term);
        }
    }

    restore_flags(flags);
}

/* write_char_current
 *   DESCRIPTION: Write a character onto the screen. Adjust the buffer values accordingly,
 *                set the cursor and handle scrolling and backspaces.
 *   INPUTS: char input - char to display
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS: the character gets printed
 */
void write_char_graphic(char input, int terminal){

    /* if a newline character came in */
    if(input == NEWLINE){

        /* if we are currently reading, then stop the read mode and put a \n in the buffer */
        if(gterminals[terminal].read_mode == 1 && terminal == curterm){
            gterminals[terminal].read_mode = 0;
            gterminals[terminal].read[gterminals[terminal].buffer_counter++] = '\n';
        }

        /* go to next line, reset the x position, increase the y position */
        gterminals[terminal].screen_x = 0;
        gterminals[terminal].screen_y++;

        /* if the screen_y position goes out of the screen, we need to scroll */
        if(gterminals[terminal].screen_y >= NUM_ROWS){
            /* scroll the screen */
            scroll_up_graphic(terminal);
        }
    }

    /* if a backspace character came in */
    else if(input == BACKSPACE){

        /* if we are currently reading */
        if(gterminals[terminal].read_mode == 1 && terminal == curterm){

            /* if the read buffer is already empty do nothing */
            if(gterminals[terminal].r1_mode){
                gterminals[terminal].read_mode = 0;
                gterminals[terminal].read[gterminals[terminal].buffer_counter++] = (char)1;
            }
            else if(gterminals[terminal].buffer_counter == 0){
                gterminals[terminal].buffer_counter = 0;
                return;
            }
            /* else remove the current head from the read buffer */
            else
                gterminals[terminal].read[--(gterminals[terminal].buffer_counter)] = '\0';
        }


        /* make the x position go back */
       gterminals[terminal].screen_x--;

        /* if the x goes beyond the screen and we are not in the left corner */
        if(gterminals[terminal].screen_x < 0 && gterminals[terminal].screen_y > 0){

            /* move up */
            gterminals[terminal].screen_y--;

            /* go the most right you can and then move backwards till you get a character */
            gterminals[terminal].screen_x = NUM_COLS - 1;

            /* make the current last character NULL */
            uint32_t start = TERMINAL_0_START_OFFSET + terminal * TERMINAL_SIZE + (gterminals[terminal].screen_y * NUM_COLS * 16 + gterminals[terminal].screen_x);
            blt_operation_mmio(0, 0, 0, 15, 80, 1, start, FONTDATAFBA, 0, 0, BLT_DST_ROP, 0, 0);
            // char * start = (char *)(0xFC000000 + TERMINAL_0_START_OFFSET + terminal * TERMINAL_SIZE + (gterminals[terminal].screen_y * NUM_COLS * 16 + gterminals[terminal].screen_x));
            // char * buffer = (char *)font_data[0];
            // int x;
            // for(x = 0; x < 16; x++){
            //     *(start + x*80) = buffer[x];
            // }

            /* clear the buffer character */
            while( 0 == is_null_graphic(gterminals[terminal].screen_x - 1, gterminals[terminal].screen_y, terminal)) { // *(uint8_t *)(video_mem + ((NUM_COLS * current->screen_y + (current->screen_x - 1)) << 1)) == '\0'){
                gterminals[terminal].screen_x--;
                /* if there is nothing on the line, then stay at the left of that line */
                if(gterminals[terminal].screen_x <= 0){
                    gterminals[terminal].screen_x = 0;
                    break;
                }
            }
        }
        /* if in the top left, just stay there */
        else if(gterminals[terminal].screen_x < 0){
            gterminals[terminal].screen_x = 0;
        }

        /* otherwise, remove a character from the buffer and write null on the screen */
        else{
            uint32_t start = TERMINAL_0_START_OFFSET + terminal * TERMINAL_SIZE + (gterminals[terminal].screen_y * NUM_COLS * 16 + gterminals[terminal].screen_x);
            blt_operation_mmio(0, 0, 0, 15, 80, 1, start, FONTDATAFBA, 0, 0, BLT_DST_ROP, 0, 0);
            // char * start = (char *)(0xFC000000 + TERMINAL_0_START_OFFSET + terminal * TERMINAL_SIZE + (gterminals[terminal].screen_y * NUM_COLS * 16 + gterminals[terminal].screen_x));
            // char * buffer = (char *)font_data[0];
            // int x;
            // for(x = 0; x < 16; x++){
            //     *(start + x*80) = buffer[x];
            // }
        }
    }

    /* for any other character */
    else{
        /* if we are reading */
        if(gterminals[terminal].read_mode == 1 && terminal == curterm){
            /* if we still have room to read, put character in the read buffer */
            if(gterminals[terminal].r1_mode == 1){
                gterminals[terminal].read_mode = 0;
                gterminals[terminal].read[gterminals[terminal].buffer_counter++] = input;
            }
            else if(gterminals[terminal].buffer_counter < MAX_READ - 1){
                gterminals[terminal].read[gterminals[terminal].buffer_counter++] = input;
            }
        }

        /* put the character on the screen and the buffer */
        uint8_t index = (uint8_t) input;
        uint32_t start = TERMINAL_0_START_OFFSET + terminal * TERMINAL_SIZE + (gterminals[terminal].screen_y * NUM_COLS * 16 + gterminals[terminal].screen_x);
        blt_operation_mmio(0, 0, 0, 15, 80, 1, start, FONTDATAFBA + index * 16, 0, 0, BLT_DST_ROP, 0, 0);
        // char * start = (char *)(0xFC000000 + TERMINAL_0_START_OFFSET + terminal * TERMINAL_SIZE + (gterminals[terminal].screen_y * NUM_COLS * 16 + gterminals[terminal].screen_x));
        // char * buffer = (char *)font_data[index];
        // int x;
        // for(x = 0; x < 16; x++){
        //     *(start + x*80) = buffer[x];
        // }
        gterminals[terminal].screen_x++;

        /* if we have gone beyond the screen, move down*/
        if(gterminals[terminal].screen_x >= NUM_COLS){
            /* reset x to left */
            gterminals[terminal].screen_x = 0;
            gterminals[terminal].screen_y++;
            /* if moving down causes you to go out, then scroll */
            if(gterminals[terminal].screen_y >= NUM_ROWS){
            /* increment the scroll counter */
            scroll_up_graphic(terminal);
            }
        }
    }

}

int is_null_graphic(int x, int y, int terminal){

    int i;
    char * start = (char *)(vidloc + TERMINAL_0_START_OFFSET + terminal * TERMINAL_SIZE + (y * NUM_COLS * 16 + x));
    for(i = 0; i < 16; i++){

        if(*(start + i*80) != 0x00) return -1;

    }

    return 0;

}

void scroll_up_graphic(int terminal){

    uint32_t start = TERMINAL_0_START_OFFSET + terminal * TERMINAL_SIZE;
    blt_operation_mmio(0, 0, 80 - 1, 24*16 - 1, 80, 80, start, start + 80*16, 0, 0, BLT_DST_ROP, 0, 0);
    blt_operation_mmio(0, 0, 80 - 1, 16 - 1, 80, 80, start + 80*16*24, 0, 0, 0, 0, 0, 0);
    gterminals[terminal].screen_y = NUM_ROWS - 1;
    gterminals[terminal].screen_x = 0;

}

/* read_buffer
 *   DESCRIPTION: read the input values from the terminal till a newline. Max size of read is 128.
 *   INPUTS: void * buffer - buffer to write to
 *           int32_t nbytes - number of bytes to read
 *   OUTPUTS: None
 *   RETURN VALUE: None
 *   SIDE EFFECTS:  The cursor gets set at the current x,y values
 */
int32_t read_graphic_terminal(void * buffer, int32_t nbytes){

    /* check for NULL buffer */
    if(buffer == NULL){
        return 0;
    }

    /* make read mode 1 */
    gterminals[process_term].read_mode = 1;

    /* write char will set this to zero when new line entered */
    while(gterminals[process_term].read_mode == 1);

    if(gterminals[process_term].r1_mode){

        *((uint8_t *)(buffer)) = (uint8_t)gterminals[process_term].read[0];
        gterminals[process_term].buffer_counter = 0;
        gterminals[process_term].read[0] = '\0';
        return 1;

    }

    /* if len of buffer is less than how much has been read, then read till len of buffer */
    int limit = 0;
    if(nbytes > gterminals[process_term].buffer_counter)
        limit = gterminals[process_term].buffer_counter;
    else
        limit = nbytes;

    /* put read buffer into input buffer */
    int i = 0;
    for(i = 0; i < limit; i++){
        *((uint8_t *)(buffer + i)) = (uint8_t)gterminals[process_term].read[i];
    }

    /* the last character should always be new line */
    *((uint8_t *)(buffer + limit - 1)) = (uint8_t)('\n');

    uint32_t main_arg_counter = set_args_graphic(buffer, limit);

    /* reset the read buffer */
    for(i = 0; i < gterminals[process_term].buffer_counter; i++){
        gterminals[process_term].read[i] = '\0';
    }

    /* reset current */
    gterminals[process_term].buffer_counter = 0;

    return main_arg_counter;
}

uint32_t set_args_graphic(uint8_t* buf, int32_t length) {

  if (length <= 0) {
    return 0;
  }

  g_args[0] = '\0';

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
  g_arg_counter = 0;
  i++;

  while (i < length - 1) {
    g_args[g_arg_counter++] = buf[i++];
  }
  g_args[g_arg_counter] = '\0';

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
int32_t write_graphic_terminal(const void * buf, int32_t nbytes){

    /* check for NULL buffer */
    if(buf == NULL){
        return 0;
    }

    int i;
    /* write the buffer character by character */
    for(i = 0; i < nbytes; i++){
        char c =  *((char *)(buf) + i);
        write_char_graphic_helper(c);
    }

    return nbytes;
}

void redraw_screen_graphic(){

    int i;
    for(i = 0; i < NUM_GRAPH_TERMINALS; i++){

        if(gterminals[i].executed == 0) continue;
        uint32_t start = TERMINAL_0_START_OFFSET + i*TERMINAL_SIZE;
        blt_operation_mmio(0xFFFFFFFF, 0xFEFEFEFE, 80*8 - 1, 25*16 - 1, 1280, 80*4, 1024*1280+(80*8*(i%2) + 20) + (i/2)*(25*16+25)*(1280), start, 0, BLT_COLOR_EXPANSION | 0x01, BLT_DST_ROP, 0, 0);

    }

}


int32_t graphic_printf(int8_t * format, ...){

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
                            write_char_graphic_helper('%');
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
                                    write_graphic_terminal(conv_buf, strlen(conv_buf));
                                } else {
                                    int32_t starting_index;
                                    int32_t i;
                                    itoa(*((uint32_t *)esp), &conv_buf[8], 16);
                                    i = starting_index = strlen(&conv_buf[8]);
                                    while(i < 8) {
                                        conv_buf[i] = '0';
                                        i++;
                                    }
                                    write_graphic_terminal(&conv_buf[starting_index], strlen(&conv_buf[starting_index]));
                                }
                                esp++;
                            }
                            break;

                        /* Print a number in unsigned int form */
                        case 'u':
                            {
                                int8_t conv_buf[36];
                                itoa(*((uint32_t *)esp), conv_buf, 10);
                                write_graphic_terminal(conv_buf, strlen(conv_buf));
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
                                write_graphic_terminal(conv_buf, strlen(conv_buf));
                                esp++;
                            }
                            break;

                        /* Print a single character */
                        case 'c':
                            write_char_graphic_helper((uint8_t) *((int32_t *)esp));
                            esp++;
                            break;

                        /* Print a NULL-terminated string */
                        case 's':
                            write_graphic_terminal(*((int8_t **)esp), strlen(*((int8_t **)esp)));
                            esp++;
                            break;

                        default:
                            break;
                    }

                }
                break;

            default:
                write_char_graphic_helper(*buf);
                break;
        }
        buf++;
    }
    return (buf - format);

}

void reset_graphic_term(int term){

    uint32_t start = TERMINAL_0_START_OFFSET + term * TERMINAL_SIZE;
    blt_operation_mmio(0, 0, 80 - 1, 25*16 - 1, 80, 80, start, 0, 0, 0, 0, 0, 0);
    memset(gterminals[term].read, 0, 128);
    gterminals[term].screen_x = 0;
    gterminals[term].screen_y = 0;

}

int switch_graphic_terminal(int newterm) {

  unsigned long flags;
  cli_and_save(flags);

  if (curterm == newterm) {
    restore_flags(flags);
    return 0;
  }

  bring_front(&(gui_root.children), &(gui_root.children_tail), newterm);

  if(gterminals[newterm].executed == 0){

    prevterm = curterm;
    curterm = newterm;
    restore_flags(flags);
    return 1;

  }

  curterm = newterm;
  restore_flags(flags);
  return 0;

}

void render_graphic_terminal(void * element){

    gui_element_t * t = (gui_element_t *) element;
    int i = t->id - 1;
    graphic_terminal_t terminal = gterminals[i];
    if(terminal.executed == 0) return;

    int x = t->x;
    int y = t->y;

    uint32_t start = TERMINAL_0_START_OFFSET + i*TERMINAL_SIZE;

    blt_operation_mmio(0, 0, BAR_WIDTH - 1, BAR_HEIGHT - 1, SCREEN_WIDTH, BAR_WIDTH, DRAWINGBOARDFBA + x + SCREEN_WIDTH*y, BARFBA, 0, 0, BLT_DST_ROP, 0, 0);
    blt_operation_mmio(gterminals[i].background, gterminals[i].text, TERMINAL_WIDTH - 1, TERMINAL_HEIGHT - BAR_HEIGHT - 1, SCREEN_WIDTH, TERMINAL_WIDTH, DRAWINGBOARDFBA + x + (y+BAR_HEIGHT)*SCREEN_WIDTH, start, 0, BLT_COLOR_EXPANSION, BLT_DST_ROP, 0, 0);

}

void set_graphic_terminal_title(int term, char * title){

    char * start = (char *)(0xFC000000 + TERMINAL_0_START_OFFSET + term * TERMINAL_SIZE + GRAPHICAL_TERMINAL_TITLE_OFFSET);
    int i;
    for(i = 0; i < 48; i++){

        uint8_t index = (uint8_t) (title[i]);
        char * buffer = (char *)font_data[index];
        
        int x;
        for(x = 0; x < 16; x++){

            *(start + x*48) = buffer[x];

        }

        start++;

    }

}

void set_terminal_bitmap(uint32_t index, int val){

    if(val == 1)
        terminal_bitmap |= (0x01 << index);
    else
        terminal_bitmap &= (~(0x01 << index));

}

int get_terminal_bitmap(uint32_t index){

    if(terminal_bitmap & (0x01 << index)) return 1;
    else return 0;

}

int find_free_terminal(){

    uint32_t i = 0;
    unsigned long flags;
    cli_and_save(flags);

    for(i = 0; i < 10; i++){

        if(get_terminal_bitmap(i) == 0){
            set_terminal_bitmap(i, 1);
            restore_flags(flags);
            return i;
        }

    }
    
    return -1;
    restore_flags(flags);

}

void free_terminal (uint32_t index){

    unsigned long flags;
    cli_and_save(flags);

    set_terminal_bitmap(index, 0);

    restore_flags(flags);
}
