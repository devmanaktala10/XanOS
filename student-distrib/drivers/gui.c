#include "gui.h"

static int last_alloc_gui;


/* gui_init
 * DESCRIPTION: Initializes the gui
*/
void gui_init(){

    page_dir[GUI_PAGE].p = 0x1;
    page_dir[GUI_PAGE].ps = 0x1;
    page_dir[GUI_PAGE].base_address = (uint32_t)GUI_PAGE_BASE_ADDRESS;
    set_cr((void *)page_dir);

    last_alloc_gui = 0;
    non_terminal_id = STATUSBARID + 1;

    int i = 0;
    for(i = 0; i < GUI_BITMAP_LENGTH; i++) gui_alloc_bitmap[i] = 0x00;

    gui_root.id = 0;
    gui_root.x = 0;
    gui_root.y = 0;
    gui_root.width = SCREEN_WIDTH;
    gui_root.height = SCREEN_HEIGHT;
    gui_root.type = TYPE_BACKGROUND;
    strncpy(gui_root.fname, "bg.txt", strlen("bg.txt"));
    gui_root.num_children = 0;
    gui_root.num_elements = 0;
    gui_root.parent = NULL;
    gui_root.elements = NULL;
    gui_root.children = NULL;
    gui_root.children_tail = NULL;
    gui_root.elements_tail = NULL;
    gui_root.render = RENDER_DEFAULT | RENDER_LOADED;
    gui_root.render_func = NULL;
    gui_root.fb_address = BACKGROUNDFBA;
    gui_root.next = NULL;
    gui_root.prev = NULL;

    gui_element_t * desk_term = alloc_gui_element();
    desk_term->id = non_terminal_id++;
    desk_term->x = 50;
    desk_term->y = 50;
    desk_term->width = 75;
    desk_term->height = 100;
    desk_term->type = TYPE_BACKGROUND | TYPE_CLICKABLE | TYPE_ELEMENT;
    reset_buffer(desk_term->fname);
    strncpy(desk_term->fname, "/utils/shell", strlen("/utils/shell"));
    desk_term->num_children = 0;
    desk_term->click_handler = desktop_icon_click_handler;
    desk_term->num_elements = 0;
    desk_term->parent = &gui_root;
    desk_term->elements = NULL;
    desk_term->children = NULL;
    desk_term->children_tail = NULL;
    desk_term->elements_tail = NULL;
    desk_term->render = RENDER_FUNC | RENDER_LOADED;
    desk_term->render_func = render_desktop_icon;
    desk_term->fb_address = TERMINALICONFBA;
    desk_term->next = NULL;
    desk_term->prev = NULL;

    gui_element_t * desk_ping = alloc_gui_element();
    desk_ping->id = non_terminal_id++;
    desk_ping->x = 50;
    desk_ping->y = 150;
    desk_ping->width = 75;
    desk_ping->height = 100;
    desk_ping->type = TYPE_BACKGROUND | TYPE_CLICKABLE | TYPE_ELEMENT;
    reset_buffer(desk_ping->fname);
    strncpy(desk_ping->fname, "/progs/pingpong", strlen("/progs/pingpong"));
    desk_ping->num_children = 0;
    desk_ping->click_handler = desktop_icon_click_handler;
    desk_ping->num_elements = 0;
    desk_ping->parent = &gui_root;
    desk_ping->elements = NULL;
    desk_ping->children = NULL;
    desk_ping->children_tail = NULL;
    desk_ping->elements_tail = NULL;
    desk_ping->render = RENDER_FUNC | RENDER_LOADED;
    desk_ping->render_func = render_desktop_icon;
    desk_ping->fb_address = PINGPONGICONFBA;
    desk_ping->next = NULL;
    desk_ping->prev = NULL;

    gui_element_t * desk_count = alloc_gui_element();
    desk_count->id = non_terminal_id++;
    desk_count->x = 50;
    desk_count->y = 250;
    desk_count->width = 75;
    desk_count->height = 100;
    desk_count->type = TYPE_BACKGROUND | TYPE_CLICKABLE | TYPE_ELEMENT;
    reset_buffer(desk_count->fname);
    strncpy(desk_count->fname, "/progs/counter", strlen("/progs/counter"));
    desk_count->num_children = 0;
    desk_count->click_handler = desktop_icon_click_handler;
    desk_count->num_elements = 0;
    desk_count->parent = &gui_root;
    desk_count->elements = NULL;
    desk_count->children = NULL;
    desk_count->children_tail = NULL;
    desk_count->elements_tail = NULL;
    desk_count->render = RENDER_FUNC | RENDER_LOADED;
    desk_count->render_func = render_desktop_icon;
    desk_count->fb_address = COUNTERICONFBA;
    desk_count->next = NULL;
    desk_count->prev = NULL;

    gui_element_t * desk_fish = alloc_gui_element();
    desk_fish->id = non_terminal_id++;
    desk_fish->x = 50;
    desk_fish->y = 350;
    desk_fish->width = 75;
    desk_fish->height = 100;
    desk_fish->type = TYPE_BACKGROUND | TYPE_CLICKABLE | TYPE_ELEMENT;
    reset_buffer(desk_fish->fname);
    strncpy(desk_fish->fname, "/progs/fish/fish", strlen("/progs/fish/fish"));
    desk_fish->num_children = 0;
    desk_fish->click_handler = desktop_icon_click_handler;
    desk_fish->num_elements = 0;
    desk_fish->parent = &gui_root;
    desk_fish->elements = NULL;
    desk_fish->children = NULL;
    desk_fish->children_tail = NULL;
    desk_fish->elements_tail = NULL;
    desk_fish->render = RENDER_FUNC | RENDER_LOADED;
    desk_fish->render_func = render_desktop_icon;
    desk_fish->fb_address = FISHICONFBA;
    desk_fish->next = NULL;
    desk_fish->prev = NULL;

    append_element(&(gui_root.elements), &(gui_root.elements_tail), desk_term);
    append_element(&(gui_root.elements), &(gui_root.elements_tail), desk_fish);
    append_element(&(gui_root.elements), &(gui_root.elements_tail), desk_count);
    append_element(&(gui_root.elements), &(gui_root.elements_tail), desk_ping);

    status_root.id = 11;
    status_root.x = 0;
    status_root.y = STATUSBAR_Y;
    status_root.width = STATUSBAR_WIDTH;
    status_root.height = STATUSBAR_HEIGHT;
    status_root.type = TYPE_BACKGROUND;
    strncpy(status_root.fname, "status.txt", strlen("status.txt"));
    status_root.num_children = 0;
    status_root.num_elements = 0;
    status_root.parent = NULL;
    status_root.elements = NULL;
    status_root.children = NULL;
    status_root.children_tail = NULL;
    status_root.elements_tail = NULL;
    status_root.render = RENDER_FUNC | RENDER_LOADED;
    status_root.render_func = render_status;
    status_root.fb_address = STATUSBARFBA;
    status_root.next = NULL;
    status_root.prev = NULL;

}

void render_desktop_icon(gui_element_t * element){

    blt_operation_mmio(0, 0, element->width - 1, 75 - 1, SCREEN_WIDTH, element->width, DRAWINGBOARDFBA + element->x + element->y * SCREEN_WIDTH, element->fb_address, 0, BLT_ENABLE_TRANSPARENCY , BLT_DST_ROP, 0, 0x3434);

    char name[32];
    parse_path(element->fname, 0, name);
    uint32_t x = element->x + element->width/2 - strlen(name)*4;
    int i;
    for(i = 0; i < strlen(name); i++){

        uint8_t index = (uint8_t)name[i];
        //make first letter capital
        if(i == 0)
            index -= 32;

        blt_operation_mmio(COLOR_EXPAND_WHITE, COLOR_EXPAND_WHITE, 7, 15, 1280, 8, DRAWINGBOARDFBA + x + (element->y + 80)*SCREEN_WIDTH, FONTDATAFBA + index * 16, 0, BLT_ENABLE_TRANSPARENCY | BLT_COLOR_EXPANSION, BLT_DST_ROP, 0, 0);
        x+=8;
        //blt_operation_mmio(COLOR_EXPAND_WHITE, COLOR_EXPAND_BLACK, t->width - 1, t->height - 1, SCREEN_WIDTH, t->width, DRAWINGBOARDFBA + x + y*SCREEN_WIDTH, start, 0, BLT_COLOR_EXPANSION | BLT_ENABLE_TRANSPARENCY, BLT_DST_ROP, 0, 0);

    }

}

void load_gui_assets(){

    int i;
    void * loc = (void *)0xFC000000;

    uint8_t m[128];
    dentry_ext_t den;
    find_dentry_ext("mouse.txt", &den, 9);
    read_data_ext(den.inode, 0, m, 128);
    memcpy(loc + MOUSEFBA, m, 128);

    uint8_t b[1280];
    find_dentry_ext("bg.txt", &den, 9);
    for(i = 0; i < 1280; i++){

        read_data_ext(den.inode, i*1024, b, 1024);
        memcpy(loc + BACKGROUNDFBA + i*1024, b, 1024);

    }

    find_dentry_ext("terminal.txt", &den, 9);
    for(i = 0; i < 25; i++){

        read_data_ext(den.inode, i*640, b, 640);
        memcpy(loc + BARFBA + i*640, b, 640);

    }

    find_dentry_ext("status.txt", &den, 9);
    for(i = 0; i < 25; i++){

        read_data_ext(den.inode, i*1280, b, 1280);
        memcpy(loc + STATUSBARFBA + i*1280, b, 1280);

    }

    find_dentry_ext("terminalicon.txt", &den, 9);
    for(i = 0; i < 75; i++){

        read_data_ext(den.inode, i*75, b, 75);
        memcpy(loc + TERMINALICONFBA + i*75, b, 75);

    }

    find_dentry_ext("countericon.txt", &den, 9);
    for(i = 0; i < 75; i++){

        read_data_ext(den.inode, i*75, b, 75);
        memcpy(loc + COUNTERICONFBA + i*75, b, 75);

    }

    find_dentry_ext("pingpongicon.txt", &den, 9);
    for(i = 0; i < 75; i++){

        read_data_ext(den.inode, i*75, b, 75);
        memcpy(loc + PINGPONGICONFBA + i*75, b, 75);

    }

    find_dentry_ext("fishicon.txt", &den, 9);
    for(i = 0; i < 75; i++){

        read_data_ext(den.inode, i*75, b, 75);
        memcpy(loc + FISHICONFBA + i*75, b, 75);

    }

    memcpy(loc + FONTDATAFBA, font_data, 256*16);

    find_dentry_ext("50x50.txt", &den, 9);
    for(i = 0; i < 50; i++){

        read_data_ext(den.inode, i*50, b, 50);
        char b2[50];
        char b3[50];
        char b4[50];
        int j;
        for(j = 0; j < 50; j++){

            if((uint8_t)b[j] == 66){
                b2[j] = 255;
                b3[j] = 69;
                b4[j] = 71;
            }
            else if((uint8_t)b[j] == 67){
                b2[j] = 68;
                b3[j] = 70;
                b4[j] = 72;
            }
            else{
                b2[j] = b[j];
                b3[j] = b[j];
                b4[j] = b[j];
            }

        }
        memcpy(loc + D50X50RED + i*50, b, 50);
        memcpy(loc + D50X50GRAY + i*50, b, 50);
        memcpy(loc + D50X50BLUE + i*50, b, 50);
        memcpy(loc + D50X50YELLOW + i*50, b, 50);

    }

    find_dentry_ext("100x50.txt", &den, 9);
    for(i = 0; i < 50; i++){

        read_data_ext(den.inode, i*100, b, 100);
        char b2[100];
        char b3[100];
        char b4[100];
        int j;
        for(j = 0; j < 100; j++){

            if((uint8_t)b[j] == 66){
                b2[j] = 255;
                b3[j] = 69;
                b4[j] = 71;
            }
            else if((uint8_t)b[j] == 67){
                b2[j] = 68;
                b3[j] = 70;
                b4[j] = 72;
            }
            else{
                b2[j] = b[j];
                b3[j] = b[j];
                b4[j] = b[j];
            }

        }
        memcpy(loc + D100X50RED + i*100, b, 100);
        memcpy(loc + D100X50GRAY + i*100, b, 100);
        memcpy(loc + D100X50BLUE + i*100, b, 100);
        memcpy(loc + D100X50YELLOW + i*100, b, 100);

    }

    find_dentry_ext("25x25.txt", &den, 9);
    for(i = 0; i < 25; i++){

        read_data_ext(den.inode, i*25, b, 25);
        char b2[25];
        char b3[25];
        char b4[25];
        int j;
        for(j = 0; j < 25; j++){

            if((uint8_t)b[j] == 66){
                b2[j] = 255;
                b3[j] = 69;
                b4[j] = 71;
            }
            else if((uint8_t)b[j] == 67){
                b2[j] = 68;
                b3[j] = 70;
                b4[j] = 72;
            }
            else{
                b2[j] = b[j];
                b3[j] = b[j];
                b4[j] = b[j];
            }

        }
        memcpy(loc + D25X25RED + i*25, b, 25);
        memcpy(loc + D25X25GRAY + i*25, b, 25);
        memcpy(loc + D25X25BLUE + i*25, b, 25);
        memcpy(loc + D25X25YELLOW + i*25, b, 25);

    }

    find_dentry_ext("50x25.txt", &den, 9);
    for(i = 0; i < 25; i++){

        read_data_ext(den.inode, i*50, b, 50);
        char b2[50];
        char b3[50];
        char b4[50];
        int j;
        for(j = 0; j < 50; j++){

            if((uint8_t)b[j] == 66){
                b2[j] = 255;
                b3[j] = 69;
                b4[j] = 71;
            }
            else if((uint8_t)b[j] == 67){
                b2[j] = 68;
                b3[j] = 70;
                b4[j] = 72;
            }
            else{
                b2[j] = b[j];
                b3[j] = b[j];
                b4[j] = b[j];
            }

        }
        memcpy(loc + D50X25RED + i*50, b, 50);
        memcpy(loc + D50X25GRAY + i*50, b, 50);
        memcpy(loc + D50X25BLUE + i*50, b, 50);
        memcpy(loc + D50X25YELLOW + i*50, b, 50);

    }

    // start loading button assets from here
    // memset(loc + 4*1024*1024-1-16000 - 1280*25 - 4*75*75 - 100*50, 0, 50*50);
}

/* alloc_gui_element()
 * DESCRIPTION: allocate and return pointer to new gui element if you can
 * RETURN VALUE: return pointer on success or NULL on failure
 */
gui_element_t * alloc_gui_element(){

    unsigned long flags;
    cli_and_save(flags);

    int i;
    uint32_t start = last_alloc_gui;
    for(i = 0; i < MAX_GUI_ELEMENTS; i++){

        uint32_t index = start/8;
        uint32_t shift = start%8;

        if((gui_alloc_bitmap[index] & (0x01 << shift)) == 0){

            gui_alloc_bitmap[index] = gui_alloc_bitmap[index] | (0x01 << shift);
            last_alloc++;
            restore_flags(flags);
            return (gui_element_t *)(ALLOC_START + sizeof(gui_element_t) * start);

        }

        start++;
        start = start%MAX_GUI_ELEMENTS;

    }

    restore_flags(flags);
    return NULL;

}

/* Add element to the tail of a doubly linked list with head and tail pointer */
void append_element(gui_element_t ** head, gui_element_t ** tail, gui_element_t * elem){

    if(*head == NULL){
        *head = elem;
        *tail = elem;
        elem->prev = NULL;
        elem->next = NULL;
        return;
    }

    (*tail)->next = elem;
    elem->next = NULL;
    elem->prev = (*tail);
    *tail = elem;

}

/* remove element by if of a doubly linked list with head and tail pointer */
void remove_element(gui_element_t ** head, gui_element_t ** tail, int id){

    // delete head
    if(*head != NULL && (*head)->id == id){

        gui_element_t * temp = *head;
        if((*head)->next == NULL){// head is only element

            *head = NULL;
            *tail = NULL;

        }
        else{

            *head = (*head)->next;
            if(*head != NULL)
            (*head)->prev = NULL;

        }
        free_gui_element(temp);
        return;
    }

    gui_element_t * prev = NULL;
    gui_element_t * c = *head;

    while(c != NULL && c->id != id){

        prev = c;
        c = c->next;

    }

    if(c == NULL) return;

    gui_element_t * temp = c;
    if(c->next == NULL){ // removing tail

        *tail = prev;
        prev->next = NULL;

    }
    else{

        prev->next = c->next;
        if(c->next != NULL)
            c->next->prev = prev;

    }

    free_gui_element(temp);

}

/* find element by id in list */
gui_element_t * find_element_by_id(gui_element_t * head, int id){

    while(head != NULL){

        if(head->id == id) return head;
        head = head->next;

    }

    return NULL;

}

/* remove element by if of a doubly linked list with head and tail pointer */
void remove_element_no_free(gui_element_t ** head, gui_element_t ** tail, int id){

    // delete head
    if(*head != NULL && (*head)->id == id){

        gui_element_t * temp = *head;
        if((*head)->next == NULL){// head is only element

            *head = NULL;
            *tail = NULL;

        }
        else{

            *head = (*head)->next;
            if(*head != NULL)
            (*head)->prev = NULL;

        }
        return;
    }

    gui_element_t * prev = NULL;
    gui_element_t * c = *head;

    while(c != NULL && c->id != id){

        prev = c;
        c = c->next;

    }

    if(c == NULL) return;

    gui_element_t * temp = c;
    if(c->next == NULL){ // removing tail

        *tail = prev;
        prev->next = NULL;

    }
    else{

        prev->next = c->next;
        if(c->next != NULL)
            c->next->prev = prev;

    }

}

/* free the space of allocated gui element */
void free_gui_element(gui_element_t * block){

    unsigned long flags;
    cli_and_save(flags);

    uint32_t start = (((uint32_t)block) - ALLOC_START)/(sizeof(gui_element_t));
    uint32_t index = start/8;
    uint32_t shift = start%8;

    gui_alloc_bitmap[index] = gui_alloc_bitmap[index] & (~(0x01 << shift));
    last_alloc_gui = start;

    restore_flags(flags);

}

/* render the gui */
void render_gui(){

    // first render root(background)
    render_element(&gui_root);

    gui_element_t * root_elems = gui_root.elements;

    // render desktop icons
    while(root_elems != NULL){

        render_element(root_elems);
        root_elems = root_elems->next;

    }

    // render the elements of the root: 
    // gui_element_t * c = gui_root.elements;

    // while(c != NULL){

    //     // if invisible or off, dont render
    //     if(c->render & RENDER_OFF || c->type & TYPE_INVISIBLE){
    //         c = c->next;
    //         continue;
    //     }

    //     render_element(c);
    //     c = c->next;

    // }
    
    // render the children: terminals
    gui_element_t * c = gui_root.children;

    while(c != NULL){

        // if render off
        if(c->render & RENDER_OFF || c->type & TYPE_INVISIBLE){
            c = c->next;
            continue;
        }
        // render terminals
        render_element(c);

        // render the elements of the terminal (cross, min, bar)
        gui_element_t * e = c->elements;
        while(e != NULL){

            if(!(e->render & RENDER_OFF)) render_element(e);
            e = e->next;

        }

        //render the children of the terminal (user added)
        e = c->children;
        while(e != NULL){

            if(!(e->render & RENDER_OFF))
            {

                render_element(e);
                // render the elements of the children (user added)
                gui_element_t * se = e->elements;

                while(se != NULL){

                    if(!(se->render & RENDER_OFF))
                        render_element(se);

                    se = se->next;

                }

            }
            e = e->next;

        }


        c = c->next;
    }

    // blt_operation_mmio(0, 0, 75 - 1, 75 - 1, 1280, 75, 1280*1024 + 100 + 100 * 1280, 4*1024*1024-1-16000 - 1280*25 - 75*75, 0, 0 | 0x08 , BLT_DST_ROP, 0, 0x3434);
    // blt_operation_mmio(0, 0, 75 - 1, 75 - 1, 1280, 75, 1280*1024 + 100 + 200 * 1280, 4*1024*1024-1-16000 - 1280*25 - 2*75*75, 0, 0 | 0x08 , BLT_DST_ROP, 0, 0x3434);
    // blt_operation_mmio(0, 0, 75 - 1, 75 - 1, 1280, 75, 1280*1024 + 100 + 300 * 1280, 4*1024*1024-1-16000 - 1280*25 - 3*75*75, 0, 0 | 0x08 , BLT_DST_ROP, 0, 0x3434);
    // blt_operation_mmio(0, 0, 75 - 1, 75 - 1, 1280, 75, 1280*1024 + 100 + 400 * 1280, 4*1024*1024-1-16000 - 1280*25 - 4*75*75, 0, 0 | 0x08 , BLT_DST_ROP, 0, 0x3434);

    // render status bar
    status_root.render_func(&(status_root));

    c = status_root.elements;

    while(c != NULL){
        render_element(c);
        c = c->next;
    }

    // render cursor
    blt_operation_mmio(COLOR_EXPAND_WHITE, COLOR_EXPAND_BLACK, CURSOR_SIZE - 1, CURSOR_SIZE - 1, SCREEN_WIDTH, CURSOR_SIZE, DRAWINGBOARDFBA + pos_x + pos_y * SCREEN_WIDTH, MOUSEFBA, 0, BLT_COLOR_EXPANSION | BLT_ENABLE_TRANSPARENCY, BLT_DST_ROP, 0, 0);

    // render entire drawing board onto the screen
    blt_operation_mmio(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, SCREEN_WIDTH, SCREEN_WIDTH, 0, DRAWINGBOARDFBA, 0, 0, BLT_DST_ROP, 0, 0);

}

/* render gui element */
void render_element(gui_element_t * element){

    if(!(element->render & RENDER_OFF)){

        if(element->render & RENDER_DEFAULT){

            render_default_element(element);

        }
        else if(element->render & RENDER_FUNC){

            element->render_func(element);

        }

    }

}

void render_default_element(gui_element_t * element){

    int x = 0;
    int y = 0;

    gui_element_t * temp = element;
    while(temp->parent != NULL){

        x += temp->parent->x;
        y += temp->parent->y;
        temp = temp->parent;

    }
    blt_operation_mmio(0, 0, element->width - 1, element->height - 1, SCREEN_WIDTH, element->width, DRAWINGBOARDFBA + element->x + x + (element->y + y) * SCREEN_WIDTH, element->fb_address, 0, 0, BLT_DST_ROP, 0, 0);

}

gui_element_t* check_click_on_elem(int x, int y){

    gui_element_t* a = &gui_root;

    // status bar
    gui_element_t * c = status_root.elements_tail;

    while(c != NULL){

        
        if((x >= c->id*100) && (x <= c->id*100 + 100) && (y >= STATUSBAR_Y - 55) && (y <= STATUSBAR_Y - 55 + 75))
                return c;

        c = c->prev;

    }

    // children from the top (terminals)
    gui_element_t* b = a->children_tail;

    while(b != NULL){

        // skip terminal if its invisible (no clicks anywhere)
        if(b->type & TYPE_INVISIBLE){

            b = b->prev;
            continue;

        }

        // check a click on terminal elements
        gui_element_t * c = b->elements_tail;

        while(c != NULL){

            if((x >= c->x + c->parent->x) && (x <= c->x + c->parent->x + c->width) && (y >= c->parent->y + c->y) && (y <= c->y + c->parent->y + c->height) && (c->type & TYPE_CLICKABLE))
                return c;

            c = c->prev;

        }

        // check click on terminal children
        c = b->children_tail;

        while(c != NULL){

            // gui_element_t * e = c->elements_tail;

            // while(e != NULL){

            //     if((x >= e->x + e->parent->x + e->parent->parent->x) && (x <= e->x + e->parent->x + e->parent->parent->x + e->width) && (y >= e->parent->parent->y + e->parent->y + e->y) && (y <= e->y + e->parent->y + e->parent->parent->y + e->height) && (e->type & TYPE_CLICKABLE))
            //         return e;

            //     e = e->prev;

            // }

            if((x >= c->x + c->parent->x) && (x <= c->x + c->parent->x + c->width) && (y >= c->parent->y + c->y) && (y <= c->y + c->parent->y + c->height) && (c->type & TYPE_CLICKABLE))
                return c;

            c = c->prev;

        }

        // check click on terminal itself
        if((x >= b->x) && (x <= b->x + b->width) && (y >= b->y) && (y <= b->y + b->height) && (b->type & TYPE_CLICKABLE))
            return b;

        b = b->prev;

    }

    // gui_elements

    c = gui_root.elements_tail;
    while(c != NULL){


        if((x >= c->x + c->parent->x) && (x <= c->x + c->parent->x + c->width) && (y >= c->parent->y + c->y) && (y <= c->y + c->parent->y + c->height) && (c->type & TYPE_CLICKABLE))
                return c;

        c = c->prev;

    }

    return NULL;

}

void bring_front(gui_element_t ** head, gui_element_t ** tail, int term){

    // delete head
    int id = term + 1;

    if(*head != NULL && (*head)->id == id){

        gui_element_t * temp = *head;
        if(*head == NULL){// head is only element

            return;

        }
        else{

            *head = (*head)->next;
            if(*head != NULL)
            (*head)->prev = NULL;
            append_element(head, tail, temp);
            return;

        }
    }

    gui_element_t * prev = NULL;
    gui_element_t * c = *head;

    while(c != NULL && c->id != id){

        prev = c;
        c = c->next;

    }

    if(c == NULL) return;

    if(c->next == NULL){ // removing tail

        return;

    }
    else{

        prev->next = c->next;
        if(c->next != NULL)
            c->next->prev = prev;
        append_element(head, tail, c);

    }

}

/* initialize a terminal gui element */
void terminal_element_initiliazer(gui_element_t * t, int term){

    if(t == NULL) halt(1);
    t->id = term + 1;
    t->x = 100*term;
    if(t->x + TERMINAL_WIDTH >= SCREEN_WIDTH){
        t->x = SCREEN_WIDTH - (t->x + TERMINAL_WIDTH - SCREEN_WIDTH) - TERMINAL_WIDTH;
    }
    t->y = 100*term;
    if(t->y + TERMINAL_HEIGHT >= SCREEN_HEIGHT){
        t->y = SCREEN_HEIGHT - (t->y + TERMINAL_HEIGHT - SCREEN_HEIGHT) - TERMINAL_HEIGHT;
    }

    t->width = TERMINAL_WIDTH;
    t->height = TERMINAL_HEIGHT;
    t->type = TYPE_TERMINAL | TYPE_CLICKABLE;
    strncpy(t->fname, "terminal.txt", strlen("terminal.txt"));
    t->num_children = 0;
    t->num_elements = 0;
    t->elements = NULL;
    t->children = NULL;
    t->elements_tail = NULL;
    t->children_tail = NULL;
    t->parent = &gui_root;
    t->render = RENDER_FUNC | RENDER_LOADED;
    t->render_func = render_graphic_terminal;
    t->click_handler = click_terminal_handler;
    t->fb_address = TERMINALICONFBA;
    t->next = NULL;
    t->prev = NULL;

    gui_element_t * bar = alloc_gui_element();
    gui_element_t * text = alloc_gui_element();
    gui_element_t * cross = alloc_gui_element();
    gui_element_t * min = alloc_gui_element();
    if(text == NULL || cross == NULL || min == NULL || bar == NULL){
        free_gui_element(bar);
        free_gui_element(text);
        free_gui_element(cross);
        free_gui_element(min);
        free_gui_element(t);
        halt(1);
    }

    bar->id = non_terminal_id++;
    bar->x = 0;
    bar->y = 0;
    bar->width = BAR_WIDTH;
    bar->height = BAR_HEIGHT;
    bar->type = TYPE_ELEMENT | TYPE_CLICKABLE;
    memcpy(cross->fname, "bar", strlen("bar"));
    bar->parent = t;
    bar->num_children = 0;
    bar->num_elements = 0;
    bar->elements = NULL;
    bar->children = NULL;
    bar->elements_tail = NULL;
    bar->children_tail = NULL;
    bar->render = RENDER_OFF;
    bar->click_handler = click_bar_handler;
    bar->fb_address = 0;
    bar->next = NULL;
    bar->prev = NULL;

    append_element(&(t->elements), &(t->elements_tail), bar);

    text->id = non_terminal_id++;
    text->x = TERMINAL_TEXT_X;
    text->y = TERMINAL_TEXT_Y;
    text->width = TERMINAL_TEXT_WIDTH;
    text->height = TERMINAL_TEXT_HEIGHT;
    text->type = TYPE_ELEMENT | TYPE_TEXT;
    memset(text->fname, 0, 60);
    text->parent = t;
    text->num_children = 0;
    text->num_elements = 0;
    text->elements = NULL;
    text->children = NULL;
    text->elements_tail = NULL;
    text->children_tail = NULL;
    text->render = RENDER_FUNC | RENDER_LOADED;
    text->render_func = render_terminal_text;
    text->click_handler = NULL;
    text->fb_address = 0;
    text->next = NULL;
    text->prev = NULL;

    append_element(&(t->elements), &(t->elements_tail), text);

    cross->id = non_terminal_id++;
    cross->x = CROSS_X;
    cross->y = CROSS_Y;
    cross->width = CROSS_WIDTH;
    cross->height = CROSS_HEIGHT;
    cross->type = TYPE_ELEMENT | TYPE_CLICKABLE;
    memcpy(cross->fname, "cross", strlen("cross"));
    cross->parent = t;
    cross->num_children = 0;
    cross->num_elements = 0;
    cross->elements = NULL;
    cross->children = NULL;
    cross->elements_tail = NULL;
    cross->children_tail = NULL;
    cross->render = RENDER_OFF;
    cross->click_handler = click_cross_handler;
    cross->fb_address = 0;
    cross->next = NULL;
    cross->prev = NULL;

    append_element(&(t->elements), &(t->elements_tail), cross);

    min->id = non_terminal_id++;
    min->x = MIN_X;
    min->y = MIN_Y;
    min->width = MIN_WIDTH;
    min->height = MIN_HEIGHT;
    min->type = TYPE_ELEMENT | TYPE_CLICKABLE;
    memcpy(min->fname, "min", strlen("min"));
    min->parent = t;
    min->num_children = 0;
    min->num_elements = 0;
    min->elements = NULL;
    min->children = NULL;
    min->elements_tail = NULL;
    min->children_tail = NULL;
    min->render = RENDER_OFF;
    min->click_handler = click_min_handler;
    min->fb_address = 0;
    min->next = NULL;
    min->prev = NULL;

    append_element(&(t->elements), &(t->elements_tail), min);

    t->num_elements = 4;

}

void render_terminal_text(void * element){

    gui_element_t * t = (gui_element_t *) element;
    gui_element_t * p = t->parent;

    int x = t->x + p->x;
    int y = t->y + p->y;
    int i = p->id - 1;

    uint32_t start = TERMINAL_0_START_OFFSET + i*TERMINAL_SIZE + GRAPHICAL_TERMINAL_TITLE_OFFSET;
    blt_operation_mmio(COLOR_EXPAND_WHITE, COLOR_EXPAND_BLACK, t->width - 1, t->height - 1, SCREEN_WIDTH, t->width, DRAWINGBOARDFBA + x + y*SCREEN_WIDTH, start, 0, BLT_COLOR_EXPANSION | BLT_ENABLE_TRANSPARENCY, BLT_DST_ROP, 0, 0);

    return;

}

/* destroy terminal */
void destroy_terminal(int term){

    gui_element_t * t = find_element_by_id(gui_root.children, term);
    if(t == NULL) return;

    gui_element_t ** h = &(t->elements);
    gui_element_t * temp = NULL;
    while(*h != NULL){

        temp = *h;
        free_gui_element(temp);
        *h = (*h)->next;

    }

    remove_element(&(gui_root.children), &(gui_root.children_tail), term);

}

/* click terminal */
void click_terminal_handler(void * element, void * mouse_argss){

    gui_element_t * t = (gui_element_t *)element;
    switch_graphic_terminal(t->id - 1);

}

/* click cross */
void click_cross_handler(void * element, void * mouse_args){

    gui_element_t * t = (gui_element_t *)element;
    switch_graphic_terminal(t->parent->id - 1);
    kill_terminal(t->parent->id - 1);

}

/* click bar */
void click_bar_handler(void * element, void * mouse_args){

    gui_element_t * t = (gui_element_t *)element;
    mouse_args_t * m = (mouse_args_t *)mouse_args;

    t = t->parent;
    switch_graphic_terminal(t->id - 1);
    int new_x = t->x;
    int new_y = t->y;
    new_x += m->byte1 - ((m->byte0 << 4) & 0x100);
    new_y -= m->byte2 - ((m->byte0 << 3) & 0x100);

    if(new_x < 0) t->x = 0;
    else if(new_x > 1280 - TERMINAL_WIDTH) t->x = 1280 - TERMINAL_WIDTH;
    else t->x = new_x;

    if(new_y < 0) t->y = 0;
    else if(new_y > 1024 - TERMINAL_HEIGHT) t->y = 1024 - TERMINAL_HEIGHT;
    else t->y = new_y;

}

/* click minimize */
void click_min_handler(void * element, void * mouse_args){

    gui_element_t * e = (gui_element_t *)element;

    gui_element_t * t = e->parent;

    t->type |= TYPE_INVISIBLE;

    remove_element_no_free(&(gui_root.children), &(gui_root.children_tail), t->id);

    t->render_func = render_status_element;
    t->click_handler = status_click_handler;

    append_element(&(status_root.elements), &(status_root.elements_tail), t);

}

void render_status_element(gui_element_t * element){

    int x = 100*element->id;
    blt_operation_mmio(0, 0, 75 - 1, 75 - 1, SCREEN_WIDTH, 75, DRAWINGBOARDFBA + x + (STATUSBAR_Y - 55) * SCREEN_WIDTH, element->fb_address, 0, 0 | BLT_ENABLE_TRANSPARENCY , BLT_DST_ROP, 0, 0x3434);

}


void status_click_handler(void * element, void * mouse_args){

    gui_element_t * t = (gui_element_t *)element;

    t->type &= ~(TYPE_INVISIBLE);

    remove_element_no_free(&(status_root.elements), &(status_root.elements_tail), t->id);

    t->render_func = render_graphic_terminal;
    t->click_handler = click_terminal_handler;

    append_element(&(gui_root.children), &(gui_root.children_tail), t);    


}

/* render status bar*/
void render_status(void * element){

    gui_element_t * t = (gui_element_t *) element;
    blt_operation_mmio(0, 0, t->width - 1, t->height - 1, SCREEN_WIDTH, SCREEN_WIDTH, DRAWINGBOARDFBA + t->x + t->y * SCREEN_WIDTH, t->fb_address, 0, 0 | BLT_ENABLE_TRANSPARENCY , BLT_DST_ROP, 0, 0x3434);

}

int32_t gui_open(){

    return 0;

}

int32_t gui_close(){

    return 0;

}

int32_t gui_read(void * buf, int32_t nbytes){

    return -1;

}

int32_t gui_write(const void * buf, int32_t nbytes){

    return -1;

}

int32_t gui_ioctl(unsigned long cmd, unsigned long arg){

    switch(cmd){

        case 0:
            return gui_ioctl_make_element(arg);
        case 1:
            return gui_ioctl_add_handler(arg);
        case 2:
            return gui_ioctl_clear_terminal(arg);
        case 3:
            return gui_ioctl_disable_terminal(arg);
        case 4:
            return gui_ioctl_terminal_color(arg);
        case 5:
            return gui_ioctl_terminal_text(arg);
        case 6:
            return gui_ioctl_element_text(arg);
        case 7:
            return gui_ioctl_set_r1(arg);
        default:
            return -1;

    }

}

int32_t gui_ioctl_set_r1(unsigned long arg){

    gterminals[process_pcb->term].r1_mode = 1;
    return 0;

}

int32_t gui_ioctl_terminal_color(unsigned long arg){

    switch(arg){

        case COLOR_WHITE:
            gterminals[process_pcb->term].background = COLOR_EXPAND_WHITE;
            return 0;
        case COLOR_BLACK:
            gterminals[process_pcb->term].background = COLOR_EXPAND_BLACK;
            return 0;
        case COLOR_BLUE:
            gterminals[process_pcb->term].background = COLOR_EXPAND_BLUE;
            return 0;
        case COLOR_RED:
            gterminals[process_pcb->term].background = COLOR_EXPAND_RED;
            return 0;
        case COLOR_YELLOW:
            gterminals[process_pcb->term].background = COLOR_EXPAND_YELLOW;
            return 0;
        case COLOR_GRAY:
            gterminals[process_pcb->term].background = COLOR_EXPAND_GRAY;
            return 0;
        default:
            return -1;

    }

}

int32_t gui_ioctl_terminal_text(unsigned long arg){

    switch(arg){

        case COLOR_WHITE:
            gterminals[process_pcb->term].text = COLOR_EXPAND_WHITE;
            return 0;
        case COLOR_BLACK:
            gterminals[process_pcb->term].text = COLOR_EXPAND_BLACK;
            return 0;
        case COLOR_BLUE:
            gterminals[process_pcb->term].text = COLOR_EXPAND_BLUE;
            return 0;
        case COLOR_RED:
            gterminals[process_pcb->term].text = COLOR_EXPAND_RED;
            return 0;
        case COLOR_YELLOW:
            gterminals[process_pcb->term].text = COLOR_EXPAND_YELLOW;
            return 0;
        case COLOR_GRAY:
            gterminals[process_pcb->term].text = COLOR_EXPAND_GRAY;
            return 0;
        default:
            return -1;

    }

}

int32_t gui_ioctl_clear_terminal(unsigned long arg){

    reset_graphic_term(process_pcb->term);
    return 0;

}

int32_t gui_ioctl_disable_terminal(unsigned long arg){

    gterminals[process_pcb->term].disabled = 1;
    return 0;

}

int32_t gui_ioctl_make_element(unsigned long arg){

    gui_user_element_t * e = (gui_user_element_t *)arg;
    if(e == NULL)
        return -1;

    int term = process_pcb->term;

    gui_element_t * terminal = find_element_by_id(gui_root.children, term + 1);

    if(terminal == NULL) {
        return -1;
    }

    if(e->flags >= 0 && e->flags <= 15){
        return init_default_button_helper(terminal, e);
    }

    else if(e->flags >= 16 && e->flags <= 21){
        return init_default_box_helper(terminal, e);
    } else if (e->flags == 22) {
      return init_transparent_box_helper(terminal, e);
    }
    else if (e->flags == 22) {
      return init_transparent_box_helper(terminal, e);
    }

    return -1;
}

void click_handler_helper(gui_element_t * t, mouse_args_t * mouse_args){

    enable_irq(MOUSE_IRQ);

    if(t->type & TYPE_USER){
        if(t->user_handler != NULL)
            user_click_handler(t);
    }
    else{
        if(t->click_handler != NULL)
            t->click_handler(t, mouse_args);
    }

}

void user_click_handler(gui_element_t * t){

    if(t->user_handler == NULL) return;

    if(t->type & TYPE_LOADED) return;

    unsigned long flags;
    cli_and_save(flags);

    t->type |= TYPE_LOADED;
    t->type |= TYPE_QUEUED;
    int term = t->parent->id - 1;
    pcb_t * term_pcb = schedule[term];
    bring_front(&(gui_root.children), &(gui_root.children_tail), term);

    restore_flags(flags);
    if(process_pcb->term == t->parent->id - 1){ // if element was clicked while program was executing
        execute_gui_handlers();
    }

}

void execute_gui_handlers(){

    if(process_pcb == NULL){
        return;
    }

    gui_element_t * e = process_pcb->gui_term->children_tail;

    unsigned long flags;
    cli_and_save(flags);

    while(e != NULL){

        if(e->type & TYPE_QUEUED){

            e->type &= ~(TYPE_QUEUED);
            /* store hardware context in ptregs */
            ptregs_t temp_ptregs;
            make_temp_ptregs(&temp_ptregs);

            temp_ptregs.sig_num = e->id;

            gui_handler_helper(e, &temp_ptregs);

        }

        e = e->prev;
    }

    restore_flags(flags);

}

void gui_handler_helper(gui_element_t * t, ptregs_t * temp_ptregs){

  uint32_t esp = temp_ptregs->esp;

  dentry_ext_t dentry;
  if(-1 == find_dentry_ext("guilink", &dentry, 7)) {
    // should kill program with error message
  }

  // get length of file and declare a buffer to copy data
  uint32_t length = read_length_ext(dentry.inode);

  esp -= length;

  uint8_t * buf = (uint8_t *)get_allocated_block();
  if(buf == NULL) return;

  uint32_t counter = 0;

  while(length > 0){

      uint32_t read = read_data_ext(dentry.inode, counter*1024, buf, 1024);
      memcpy((uint8_t*)esp + counter*1024, (uint8_t *)buf, read);
      length -= read;
      counter++;

  }

//   uint8_t buf[length];

  // make space for data on stack

//   read_data_ext(dentry.inode, 0, (uint8_t*) buf, length);
//   memcpy((uint8_t*) esp, (uint8_t*) buf, length);

  // get instruction pointer and modify it to point to start of siglink

  read_data_ext(dentry.inode, 0, buf, 1024);

  uint32_t eip = *(((uint32_t *)(buf) + 6));
  eip = eip - PROCESS_START + esp;

  free_allocated_block((char *)buf);

  // now push hardware context to user stack
  esp -= sizeof(ptregs_t);
  memcpy((uint8_t*) esp, temp_ptregs, sizeof(ptregs_t));

  // save pointer to hardware_context into process_pcb to access in sigreturn
  process_pcb->gui_hw_esp = esp;

  // push signal signal_number
  esp -= 4;
  memcpy((uint8_t*) esp, &(t->id), 4);

  // push eip of siglink
  esp -= 4;
  memcpy((uint8_t*) esp, &eip, 4);

  // jump to handler provided by user
  asm volatile("                      \n\
                cli                   \n\
                pushl %%eax           \n\
                pushl %%ebx           \n\
                pushf                 \n\
                popl %%eax            \n\
                orl $0x0200, %%eax    \n\
                pushl %%eax           \n\
                pushl %%ecx           \n\
                pushl %%edx           \n\
                iret                  \n\
                "
                :
                :"a"(USER_DS), "b"(esp), "c"(USER_CS), "d"(t->user_handler)
                :"memory", "cc"
  );

}

int32_t gui_ioctl_add_handler(unsigned long arg){

    gui_handler_addition_t * hand = (gui_handler_addition_t *) (arg);

    gui_element_t * elem = find_element_by_id(process_pcb->gui_term->children, hand->id);

    if(elem == NULL)
        return -1;

    elem->user_handler = hand->handler;
    elem->type |= TYPE_CLICKABLE;

    return 0;
}

/* have to remove from process pcb as well as here but can just set head and tail to null in process pcb? */
void remove_user_elements(int term){

    gui_element_t * t = find_element_by_id(gui_root.children, term);
    if(t == NULL) return;

    unsigned long flags;
    cli_and_save(flags);

    gui_element_t ** h = &(t->children);
    gui_element_t * temp = NULL;
    while(*h != NULL){

        temp = (*h)->elements;
        while(temp != NULL){

            free_gui_element(temp);
            temp = temp->next;

        }

        temp = *h;
        free_gui_element(temp);
        *h = (*h)->next;

    }

    restore_flags(flags);

}

void desktop_icon_click_handler(void * element, void * mouse_args){

    gui_element_t * t = (gui_element_t *)element;
    icon_exec = 1;
    reset_buffer(exec_path);
    memcpy(exec_path, t->fname, strlen(t->fname));

}

int32_t init_default_button_helper(gui_element_t * terminal, gui_user_element_t * elem){

    if(elem->flags%4 == 0){
        return init_default_50x50(terminal, elem);
    }
    else if(elem->flags%4 == 1){
        return init_default_100x50(terminal, elem);
    }
    else if(elem->flags%4 == 2){
        return init_default_25x25(terminal, elem);
    }
    else if(elem->flags%4 == 3){
        return init_default_50x25(terminal, elem);
    }


}

int32_t init_default_50x50(gui_element_t * terminal, gui_user_element_t * e){

    if(e->x + 50 >= TERMINAL_WIDTH || e->x < 0 || e->y + 50 > TERMINAL_HEIGHT - 25 || e->y < 0)
        return -1;

    gui_element_t * elem = alloc_gui_element();
    if(elem == NULL) return -1;

    elem->id = non_terminal_id++;
    elem->x = e->x;
    elem->y = e->y + 25;
    elem->width = 50;
    elem->height = 50;
    elem->type = TYPE_ELEMENT | TYPE_USER;
    elem->parent = terminal;
    elem->num_children = 0;
    elem->num_elements = 0;
    elem->elements = NULL;
    elem->children = NULL;
    elem->elements_tail = NULL;
    elem->children_tail = NULL;
    elem->render = RENDER_LOADED | RENDER_FUNC;
    elem->render_func = render_default_user_element;
    elem->click_handler = NULL;
    elem->next = NULL;
    elem->prev = NULL;

    if(e->flags == USER_DEFAULT_BUTTON_50_X_50_RED){
        memcpy(elem->fname, "50x50red", strlen("50x50red"));
        elem->fb_address = D50X50RED;
    }
    else if(e->flags == USER_DEFAULT_BUTTON_50_X_50_GRAY){
        memcpy(elem->fname, "50x50gray", strlen("50x50gray"));
        elem->fb_address = D50X50GRAY;
    }
    else if(e->flags == USER_DEFAULT_BUTTON_50_X_50_BLUE){
        memcpy(elem->fname, "50x50blue", strlen("50x50blue"));
        elem->fb_address = D50X50BLUE;
    }
    else if(e->flags == USER_DEFAULT_BUTTON_50_X_50_YELLOW){
        memcpy(elem->fname, "50x50yellow", strlen("50x50yellow"));
        elem->fb_address = D50X50YELLOW;
    }

    terminal->num_children++;
    append_element(&(terminal->children), &(terminal->children_tail), elem);

    return elem->id;

}

int32_t init_default_100x50(gui_element_t * terminal, gui_user_element_t * e){

    if(e->x + 100 >= TERMINAL_WIDTH || e->x < 0 || e->y + 50 > TERMINAL_HEIGHT - 25 || e->y < 0)
        return -1;

    gui_element_t * elem = alloc_gui_element();
    if(elem == NULL) return -1;

    elem->id = non_terminal_id++;
    elem->x = e->x;
    elem->y = e->y + 25;
    elem->width = 100;
    elem->height = 50;
    elem->type = TYPE_ELEMENT | TYPE_USER;
    elem->parent = terminal;
    elem->num_children = 0;
    elem->num_elements = 0;
    elem->elements = NULL;
    elem->children = NULL;
    elem->elements_tail = NULL;
    elem->children_tail = NULL;
    elem->render = RENDER_LOADED | RENDER_FUNC;
    elem->render_func = render_default_user_element;
    elem->click_handler = NULL;
    elem->next = NULL;
    elem->prev = NULL;

    if(e->flags == USER_DEFAULT_BUTTON_100_X_50_RED){
        memcpy(elem->fname, "100x50red", strlen("100x50red"));
        elem->fb_address = D100X50RED;
    }
    else if(e->flags == USER_DEFAULT_BUTTON_100_X_50_GRAY){
        memcpy(elem->fname, "100x50gray", strlen("100x50gray"));
        elem->fb_address = D100X50GRAY;
    }
    else if(e->flags == USER_DEFAULT_BUTTON_100_X_50_BLUE){
        memcpy(elem->fname, "50x50blue", strlen("50x50blue"));
        elem->fb_address = D100X50BLUE;
    }
    else if(e->flags == USER_DEFAULT_BUTTON_100_X_50_YELLOW){
        memcpy(elem->fname, "50x50yellow", strlen("50x50yellow"));
        elem->fb_address = D100X50YELLOW;
    }

    terminal->num_children++;
    append_element(&(terminal->children), &(terminal->children_tail), elem);

    return elem->id;

}

int32_t init_default_25x25(gui_element_t * terminal, gui_user_element_t * e){

    if(e->x + 25 >= TERMINAL_WIDTH || e->x < 0 || e->y + 25 > TERMINAL_HEIGHT - 25 || e->y < 0)
        return -1;

    gui_element_t * elem = alloc_gui_element();
    if(elem == NULL) return -1;

    elem->id = non_terminal_id++;
    elem->x = e->x;
    elem->y = e->y + 25;
    elem->width = 25;
    elem->height = 25;
    elem->type = TYPE_ELEMENT | TYPE_USER;
    elem->parent = terminal;
    elem->num_children = 0;
    elem->num_elements = 0;
    elem->elements = NULL;
    elem->children = NULL;
    elem->elements_tail = NULL;
    elem->children_tail = NULL;
    elem->render = RENDER_LOADED | RENDER_FUNC;
    elem->render_func = render_default_user_element;
    elem->click_handler = NULL;
    elem->next = NULL;
    elem->prev = NULL;

    if(e->flags == USER_DEFAULT_BUTTON_25_X_25_RED){
        memcpy(elem->fname, "25x25red", strlen("25x25red"));
        elem->fb_address = D25X25RED;
    }
    else if(e->flags == USER_DEFAULT_BUTTON_25_X_25_GRAY){
        memcpy(elem->fname, "25x25gray", strlen("25x25gray"));
        elem->fb_address = D25X25GRAY;
    }
    else if(e->flags == USER_DEFAULT_BUTTON_25_X_25_BLUE){
        memcpy(elem->fname, "25x25blue", strlen("25x25blue"));
        elem->fb_address = D25X25BLUE;
    }
    else if(e->flags == USER_DEFAULT_BUTTON_25_X_25_YELLOW){
        memcpy(elem->fname, "25x25yellow", strlen("25x25yellow"));
        elem->fb_address = D25X25YELLOW;
    }

    terminal->num_children++;
    append_element(&(terminal->children), &(terminal->children_tail), elem);

    return elem->id;

}

int32_t init_default_50x25(gui_element_t * terminal, gui_user_element_t * e){

    if(e->x + 50 >= TERMINAL_WIDTH || e->x < 0 || e->y + 25 > TERMINAL_HEIGHT - 25 || e->y < 0)
        return -1;

    gui_element_t * elem = alloc_gui_element();
    if(elem == NULL) return -1;

    elem->id = non_terminal_id++;
    elem->x = e->x;
    elem->y = e->y + 25;
    elem->width = 50;
    elem->height = 25;
    elem->type = TYPE_ELEMENT | TYPE_USER;
    elem->parent = terminal;
    elem->num_children = 0;
    elem->num_elements = 0;
    elem->elements = NULL;
    elem->children = NULL;
    elem->elements_tail = NULL;
    elem->children_tail = NULL;
    elem->render = RENDER_LOADED | RENDER_FUNC;
    elem->render_func = render_default_user_element;
    elem->click_handler = NULL;
    elem->next = NULL;
    elem->prev = NULL;

    if(e->flags == USER_DEFAULT_BUTTON_50_X_25_RED){
        memcpy(elem->fname, "50x25red", strlen("50x25red"));
        elem->fb_address = D50X25RED;
    }
    else if(e->flags == USER_DEFAULT_BUTTON_50_X_25_GRAY){
        memcpy(elem->fname, "50x25gray", strlen("50x25gray"));
        elem->fb_address = D50X25GRAY;
    }
    else if(e->flags == USER_DEFAULT_BUTTON_50_X_25_BLUE){
        memcpy(elem->fname, "50x25blue", strlen("50x25blue"));
        elem->fb_address = D50X25BLUE;
    }
    else if(e->flags == USER_DEFAULT_BUTTON_50_X_25_YELLOW){
        memcpy(elem->fname, "50x25yellow", strlen("50x25yellow"));
        elem->fb_address = D50X25YELLOW;
    }

    terminal->num_children++;
    append_element(&(terminal->children), &(terminal->children_tail), elem);

    return elem->id;

}

void render_default_user_element(gui_element_t * element){

    int x = 0;
    int y = 0;

    gui_element_t * temp = element;
    while(temp->parent != NULL){

        x += temp->parent->x;
        y += temp->parent->y;
        temp = temp->parent;

    }

    x += element->x;
    y += element->y;

    blt_operation_mmio(0, 0, element->width - 1, element->height - 1, SCREEN_WIDTH, element->width, DRAWINGBOARDFBA + x + y * SCREEN_WIDTH, element->fb_address, 0, BLT_ENABLE_TRANSPARENCY , BLT_DST_ROP, 0, 0x3434);

}

int32_t init_default_box_helper(gui_element_t * terminal, gui_user_element_t * e){

    if(e->x + e->width >= TERMINAL_WIDTH || e->x < 0 || e->y + e->height > TERMINAL_HEIGHT - 25 || e->y < 0)
    return -1;

    gui_element_t * elem = alloc_gui_element();
    if(elem == NULL) return -1;

    elem->id = non_terminal_id++;
    elem->x = e->x;
    elem->y = e->y + 25;
    elem->width = e->width;
    elem->height = e->height;
    elem->type = TYPE_ELEMENT | TYPE_USER;
    elem->parent = terminal;
    elem->num_children = 0;
    elem->num_elements = 0;
    elem->elements = NULL;
    elem->children = NULL;
    elem->elements_tail = NULL;
    elem->children_tail = NULL;
    elem->render = RENDER_LOADED | RENDER_FUNC;
    elem->render_func = render_default_user_box;
    elem->click_handler = NULL;
    elem->fb_address = 0;
    elem->next = NULL;
    elem->prev = NULL;

    if(e->flags == USER_DEFAULT_BOX_RED){
        memcpy(elem->fname, "boxred", strlen("boxred"));
    }
    if(e->flags == USER_DEFAULT_BOX_BLUE){
        memcpy(elem->fname, "boxblue", strlen("boxblue"));
    }
    if(e->flags == USER_DEFAULT_BOX_YELLOW){
        memcpy(elem->fname, "boxyellow", strlen("boxyellow"));
    }
    if(e->flags == USER_DEFAULT_BOX_WHITE){
        memcpy(elem->fname, "boxwhite", strlen("boxwhite"));
    }
    if(e->flags == USER_DEFAULT_BOX_BLACK){
        memcpy(elem->fname, "boxblack", strlen("boxblack"));
    }
    if(e->flags == USER_DEFAULT_BOX_GRAY){
        memcpy(elem->fname, "boxgray", strlen("boxgray"));
    }

    terminal->num_children++;
    append_element(&(terminal->children), &(terminal->children_tail), elem);

    return elem->id;

}

void render_default_user_box(gui_element_t * e){

    uint32_t color;
    if(strncmp(e->fname, "boxred", strlen("boxred")) == 0){
        color = COLOR_EXPAND_RED;
    }
    else if(strncmp(e->fname, "boxblue", strlen("boxblue")) == 0){
        color = COLOR_EXPAND_BLUE;
    }
    else if(strncmp(e->fname, "boxwhite", strlen("boxwhite")) == 0){
        color = COLOR_EXPAND_WHITE;
    }
    else if(strncmp(e->fname, "boxblack", strlen("boxblack")) == 0){
        color = COLOR_EXPAND_BLACK;
    }
    else if(strncmp(e->fname, "boxyellow", strlen("boxyellow")) == 0){
        color = COLOR_EXPAND_YELLOW;
    }
    else if(strncmp(e->fname, "boxgray", strlen("boxgray")) == 0){
        color = COLOR_EXPAND_GRAY;
    }

    int x = 0;
    int y = 0;

    gui_element_t * temp = e;
    while(temp->parent != NULL){

        x += temp->parent->x;
        y += temp->parent->y;
        temp = temp->parent;

    }

    x += e->x;
    y += e->y;

    blt_operation_mmio(0, color, e->width - 1, e->height - 1, SCREEN_WIDTH, e->width, DRAWINGBOARDFBA + x + y*SCREEN_WIDTH, 0, 0, 0x40 | BLT_COLOR_EXPANSION, BLT_DST_ROP, 0x04, 0);

}

int32_t gui_ioctl_element_text(unsigned long arg){

    gui_text_addition_t * t = (gui_text_addition_t *)arg;
    if(t == NULL) return -1;

    gui_element_t * terminal = (gui_element_t *)process_pcb->gui_term;
    gui_element_t * child = find_element_by_id(terminal->children, t->id);

    if(child == NULL)return -1;

    if(t->x + strlen(t->text)*8 > child->width || t->y + 16 > child->height) return -1;

    gui_element_t * elem = alloc_gui_element();
    if(elem == NULL) return -1;

    elem->id = non_terminal_id++;
    elem->x = t->x;
    elem->y = t->y;
    elem->width = strlen(t->text) * 8;
    elem->height = 16;
    elem->type = TYPE_ELEMENT | TYPE_USER | TYPE_TEXT;
    elem->parent = child;
    elem->num_children = 0;
    elem->num_elements = 0;
    elem->elements = NULL;
    elem->children = NULL;
    elem->elements_tail = NULL;
    elem->children_tail = NULL;
    elem->render = RENDER_LOADED | RENDER_FUNC;
    elem->render_func = render_user_text;
    elem->click_handler = NULL;
    elem->fb_address = 0;
    elem->next = NULL;
    elem->prev = NULL;

    int term = terminal->id - 1;
    int x = child->x + t->x;
    int y = child->y + t->y;

    if(t->flags == COLOR_WHITE){
        memcpy(elem->fname, "textwhit", strlen("textwhit"));
    }
    else if(t->flags == COLOR_BLACK){
        memcpy(elem->fname, "textblac", strlen("textblac"));
    }
    else if(t->flags == COLOR_RED){
        memcpy(elem->fname, "textredd", strlen("textredd"));
    }
    else if(t->flags == COLOR_YELLOW){
        memcpy(elem->fname, "textyell", strlen("textyell"));
    }
    else if(t->flags == COLOR_GRAY){
        memcpy(elem->fname, "textgray", strlen("textgray"));
    }
    else if(t->flags == COLOR_BLUE){
        memcpy(elem->fname, "textblue", strlen("textblue"));
    }

    memcpy(elem->fname + 10, t->text, 50);

    append_element(&(child->elements), &(child->elements_tail), elem);
    return elem->id;

}

void render_user_text(gui_element_t * t){

    int x = t->parent->parent->x + t->parent->x + t->x;
    int y = t->parent->parent->y + t->parent->y + t->y;

    uint32_t color;

    if(strncmp(t->fname, "textredd", strlen("textredd")) == 0){
        color = COLOR_EXPAND_RED;
    }
    else if(strncmp(t->fname, "textblue", strlen("textblue")) == 0){
        color = COLOR_EXPAND_BLUE;
    }
    else if(strncmp(t->fname, "textwhit", strlen("textwhit")) == 0){
        color = COLOR_EXPAND_WHITE;
    }
    else if(strncmp(t->fname, "textblac", strlen("textblac")) == 0){
        color = COLOR_EXPAND_BLACK;
    }
    else if(strncmp(t->fname, "textyell", strlen("textyell")) == 0){
        color = COLOR_EXPAND_YELLOW;
    }
    else if(strncmp(t->fname, "textgray", strlen("textgray")) == 0){
        color = COLOR_EXPAND_GRAY;
    }

    int i;
    for(i = 0; i < strlen(t->fname + 10); i++){

        uint8_t index = (uint8_t)(t->fname[10 + i]);
        blt_operation_mmio(0, color, 7, 15, SCREEN_WIDTH, 8, DRAWINGBOARDFBA + x + y*SCREEN_WIDTH, FONTDATAFBA + index*16, 0, BLT_COLOR_EXPANSION | BLT_ENABLE_TRANSPARENCY, BLT_DST_ROP, 0, 0);
        x += 8;

    }

}

int32_t init_transparent_box_helper(gui_element_t* terminal, gui_user_element_t* e) {
  if(e->x + e->width >= TERMINAL_WIDTH || e->x < 0 || e->y + e->height > TERMINAL_HEIGHT - 25 || e->y < 0)
  return -1;

  gui_element_t * elem = alloc_gui_element();
  if(elem == NULL) return -1;

  elem->id = non_terminal_id++;
  elem->x = e->x;
  elem->y = e->y + 25;
  elem->width = e->width;
  elem->height = e->height;
  elem->type = TYPE_ELEMENT | TYPE_USER;
  elem->parent = terminal;
  elem->num_children = 0;
  elem->num_elements = 0;
  elem->elements = NULL;
  elem->children = NULL;
  elem->elements_tail = NULL;
  elem->children_tail = NULL;
  elem->render = RENDER_OFF;
  elem->render_func = NULL;
  elem->click_handler = NULL;
  elem->fb_address = 0;
  elem->next = NULL;
  elem->prev = NULL;

  terminal->num_children++;
  append_element(&(terminal->children), &(terminal->children_tail), elem);

  return elem->id;
}
