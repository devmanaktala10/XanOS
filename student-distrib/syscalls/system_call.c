#include "system_call.h"

/* Stdin function table for open, read, write, close */
struct function_table stdin_table={
  stdin_open, read_graphic_terminal, stdin_write, stdin_close, stdin_ioctl
};

/* Stdout function table for open, read, write, close */
struct function_table stdout_table={
  stdout_open, stdout_read, write_graphic_terminal, stdout_close, stdin_ioctl
};

/* File function table for open, read, write, close */
struct function_table file_table={
  file_open, file_read, file_write, file_close, file_ioctl
};

/* RTC function table for open, read, write, close */
struct function_table rtc_table={
  rtc_open, rtc_read, rtc_write, rtc_close, rtc_ioctl
};

/* Directory function table for open, read, write, close */
struct function_table dir_table={
  dir_open, dir_read, dir_write, dir_close, dir_ioctl
};

struct function_table gui_table={
  gui_open, gui_read, gui_write, gui_close, gui_ioctl
};

struct function_table sb16_table={
  sb16_open, sb16_read, sb16_write, sb16_close, sb16_ioctl
};

struct function_table net_table={
  net_open, net_read, net_write, net_close, net_ioctl
};

/* load_system_call
 *   DESCRIPTION: initialises interrupts from a specific irq number
 *   INPUTS: irq - an irq struct containing info of the irq handler and number
 *   OUTPUTS: void
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes the irq struct array. Enables irq on the master/slave PIC
 */
void load_system_call() {

    idt_desc_t sys_call;
    sys_call.seg_selector = KERNEL_CS; /* Exception are handled in the Kernel */
    sys_call.reserved4 = 0;
    sys_call.reserved3 = 1;            /* One for exceptions */
    sys_call.reserved2 = 1;
    sys_call.reserved1 = 1;
    sys_call.size      = 1;
    sys_call.reserved0 = 0;
    sys_call.dpl       = 3;            /* Kernel Privelege */
    sys_call.present   = 1;
    SET_IDT_ENTRY(sys_call, syscall_handler);    /* Load the Exception handler */
    idt[SYS_CALL] = sys_call;                                 /* Modify IDT entry */

}

/* read
 * DESCRIPTION: System call that reads nbytes from file specified
 *              by the file descriptor fd and stores the bytes read
 *              in the input buffer. Uses the approriate jump table
 *              for the open file.
 * INPUTS: int32_t fd - file descriptor
 *         void * buf - buffer to store bytes read in
 *         int32_t nbytes - number of bytes to read
 * OUTPUTS: None
 * RETURN VALUE: int32_t number of bytes read or -1 for bad input
 * SIDE EFFECTS: sets rel_fd */
int32_t read(int32_t fd, void * buf, int32_t nbytes) {

  if(buf == NULL) return -1;

  // Check if fd is invalid and if nbytes is negative
  if (nbytes < 0 || fd < 0 || fd >= MAX_FD_LENGTH) {
    return -1;
  }

  // Get the relevant file and check if its present
  file_t* rel_file = &(process_pcb->file[fd]);
  if (rel_file->flags == 0) {
    return -1;
  }

  // modify rel_fd for file_read
  rel_fd = fd;
  // return no. of bytes written
  return rel_file->file_op->read(buf, nbytes);

}

/* write
 * DESCRIPTION: System call that writes nbytes from buffer to
 *              the file specified by the file descriptor passed in
 * INPUTS: int32_t fd - file descriptor
 *         void * buf - buffer of bytes to write
 *         int32_t nbytes - number of bytes to write
 * OUTPUTS: None
 * RETURN VALUE: int32_t number of bytes written or -1 on bad input
 * SIDE EFFECTS: None */
int32_t write(int32_t fd, const void * buf, int32_t nbytes) {

    if(buf == NULL) return -1;

    // check if fd is valid
    if (fd < 0 || fd >= MAX_FD_LENGTH) {
      return -1;
    }

    // get relevant file and check if file is present
    file_t* rel_file = &(process_pcb->file[fd]);
    if (rel_file->flags == 0) {
      return -1;
    }

    rel_fd = fd;
    return rel_file->file_op->write(buf, nbytes);
}

/* execute
 * DESCRIPTION: System call that executes a user level program
 *              passed in via the command argument. This function
 *              is responsible for setting up the child process'
 *              PCB, its necessary paging structures and its files
 * INPUTS: uint8_t * command - the name of the program to execute
 * OUTPUTS: None
 * RETURN VALUE: The return value when the executed task is halted or -1 on failure
 * SIDE EFFECTS: None */
int32_t execute(const uint8_t * command){

    unsigned long flags;
    cli_and_save(flags);
    /* store the current ebp, esp, and return address */
    uint32_t ra, ebp, esp_ex;
    asm volatile("                      \n\
                  movl (%%ebp), %%ebx   \n\
                  movl 4(%%ebp), %%ecx  \n\
                  xorl %%edx, %%edx     \n\
                  addl %%esp, %%edx     \n\
                  "
                  :"=b"(ebp), "=c"(ra), "=d"(esp_ex)
                  :
                  :"memory");

    int pid = find_free_pid();
    if(pid == -1){
      restore_flags(flags);
      return -1;
    }

    /* if the command does not exist in fs return -1 */
    dentry_ext_t dentry;
    // if(-1 == read_dentry_by_name(command, &dentry))
    //     return -1;
    if(pid != 0){
      if(-1 == find_dentry_ext((char *)command, &dentry, process_pcb->curdir)){
        if(-1 == find_dentry_ext((char *)command, &dentry, 1)){
          free_pid(pid);
          return -1;
        }
      }
    }
    else{
      if(-1 == find_dentry_ext((char *)command, &dentry, 0)){
        if(-1 == find_dentry_ext((char *)command, &dentry, 1)){
          free_pid(pid);
          return -1;
        }
      }
    }

    char buf[40];
    /* read the first 40 bytes of file */
    // read_data(dentry.inode_num, 0, (uint8_t*) buf, 40);
    read_data_ext(dentry.inode, 0, (uint8_t*)buf, 40);

    /* check for executable magic number */
    if(buf[0] == 0x7f && buf[1] == 0x45 && buf[2] == 0x4c && buf[3] == 0x46){

        /* store the process eip (bytes 24-27) and esp bottom of 132MB */
        uint32_t eip = *(((uint32_t *)(buf) + 6));
        uint32_t esp = 0x08000000 + FOURMB - 4;

        char title[48];
        memset(title, 0, 48);
        memcpy(title + ((48 - strlen((int8_t *)command))/2), command, strlen((int8_t *)command));
        set_graphic_terminal_title(curterm, title);

        /* setup the process paging structure */
        setup_process(pid, dentry.inode);

        /* assign the process pcb block to the top of its stack */
        process_pcb = (pcb_t * ) (EIGHTMB - pid * (KERNEL_STACK_SIZE) - KERNEL_STACK_SIZE);
        process_pcb->pid = pid;

        unsigned i;
        for (i = 0; i < SIGNAL_NUM; i++) {
          process_pcb->sigs.sig_ptr[i] = default_signals.sig_ptr[i];
          process_pcb->sigs.bitvector = 0;
        }


        /* get the currently executing process' pcb stack */
        pcb_t * current_pcb;
        asm volatile("                          \n\
                    xorl %%ecx, %%ecx           \n\
                    addl %%esp, %%ecx           \n\
                    andl $0xffffe000, %%ecx     \n\
                    "
                    :"=c"(current_pcb)
                    :
                    :"memory", "cc"
        );

        /* store the return value the ebp, esp and parent pcb pointer in the process' pcb */

        process_pcb->ra = ra;
        process_pcb->ebp = ebp;
        process_pcb->esp = esp_ex;
        process_pcb->pp  = current_pcb;
        process_pcb->isBaseTerm = 0x00;
        memcpy(process_pcb->pname, title, 48);

        process_pcb->gui_term = process_pcb->pp->gui_term;

        /* setup the terminal for the process */
        process_pcb->term = curterm;
        if(curterm != process_pcb->pp->term)
          process_pcb->curdir = 0;
        else
          process_pcb->curdir = process_pcb->pp->curdir;

        schedule[curterm] = process_pcb;

        // gterminals[curterm].executed = 1;

        /* setup stdin and stdout for the process */
        setup_stdinout();

        /* save all registers in the pcb */
        asm volatile("                      \n\
                      movl %%eax, 20(%%eax) \n\
                      movl %%ebx, 24(%%eax) \n\
                      movl %%ecx, 28(%%eax) \n\
                      movl %%edx, 32(%%eax) \n\
                      movl %%esi, 36(%%eax) \n\
                      movl %%edi, 40(%%eax) \n\
                      pushf                 \n\
                      popl %%ebx            \n\
                      movl %%ebx, 44(%%eax) \n\
                      "
                      :
                      :"a"(process_pcb)
                      :"memory"
        );

        /* halt status return */
        int ret;
        /* IRET into the user level program and start executing code
         * first cli to disable interrupts during critical section
         * push user data segment, user esp, user code segment, user
         * eip. Push and pop the flags into %eax and then or with 0x0200
         * to enable interrupts in the user, finally iret to start execution */
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
                      h:                    \n\
                      "
                      :"=a"(ret)
                      :"a"(USER_DS), "b"(esp), "c"(USER_CS), "d"(eip)
                      :"memory", "cc"
        );
        return ret;
    }

    /* not an executable return -1 */
    else{
        free_pid(pid);
        return -1;
    }
}

/* execute_new_term
 * DESCRIPTION: System call that executes a user level program
 *              passed in via the command argument. This function
 *              is responsible for setting up the child process'
 *              PCB, its necessary paging structures and its files
 * INPUTS: uint8_t * command - the name of the program to execute
 * OUTPUTS: None
 * RETURN VALUE: The return value when the executed task is halted or -1 on failure
 * SIDE EFFECTS: None */
int32_t execute_new_term(const uint8_t * command, uint32_t term){

    unsigned long flags;
    cli_and_save(flags);
    /* store the current ebp, esp, and return address */
    uint32_t ra, ebp, esp_ex;
    asm volatile("                      \n\
                  movl (%%ebp), %%ebx   \n\
                  movl 4(%%ebp), %%ecx  \n\
                  xorl %%edx, %%edx     \n\
                  addl %%esp, %%edx     \n\
                  "
                  :"=b"(ebp), "=c"(ra), "=d"(esp_ex)
                  :
                  :"memory");

    int pid = find_free_pid();
    if(pid == -1){
      restore_flags(flags);
      return -1;
    }

    /* if the command does not exist in fs return -1 */
    dentry_ext_t dentry;
    // if(-1 == read_dentry_by_name(command, &dentry))
    //     return -1;

    char name[32];
    reset_buffer(name);
    int ret = parse_path((char *)command, 0, name);
    if(ret == -1){
      free_pid(pid);
      return -1;
    }
    if(-1 == find_dentry_ext(name, &dentry, ret)){
      free_pid(pid);
      return -1;
    }

    // if(pid != 0){
    //   if(-1 == find_dentry_ext((char *)command, &dentry, process_pcb->curdir)){
    //     if(-1 == find_dentry_ext((char *)command, &dentry, 1)){
    //       free_pid(pid);
    //       return -1;
    //     }
    //   }
    // }
    // else{
    //   if(-1 == find_dentry_ext((char *)command, &dentry, 0)){
    //     if(-1 == find_dentry_ext((char *)command, &dentry, 1)){
    //       free_pid(pid);
    //       return -1;
    //     }
    //   }
    // }

    char buf[40];
    /* read the first 40 bytes of file */
    // read_data(dentry.inode_num, 0, (uint8_t*) buf, 40);
    read_data_ext(dentry.inode, 0, (uint8_t *)buf, 40);

    /* check for executable magic number */
    if(buf[0] == 0x7f && buf[1] == 0x45 && buf[2] == 0x4c && buf[3] == 0x46){

        /* store the process eip (bytes 24-27) and esp bottom of 132MB */
        uint32_t eip = *(((uint32_t *)(buf) + 6));
        uint32_t esp = 0x08000000 + FOURMB - 4;

        char title[48];
        memset(title, 0, 48);
        memcpy(title + ((48 - strlen((int8_t *)name))/2), name, strlen((int8_t *)command));
        set_graphic_terminal_title(term, title);

        /* setup the process paging structure */
        setup_process(pid, dentry.inode);

        num_active_terminals++;

        /* assign the process pcb block to the top of its stack */
        process_pcb = (pcb_t * ) (EIGHTMB - pid * (KERNEL_STACK_SIZE) - KERNEL_STACK_SIZE);
        process_pcb->pid = pid;

        unsigned i;
        for (i = 0; i < SIGNAL_NUM; i++) {
          process_pcb->sigs.sig_ptr[i] = default_signals.sig_ptr[i];
          process_pcb->sigs.bitvector = 0;
        }

        /* get the currently executing process' pcb stack */
        pcb_t * current_pcb;
        asm volatile("                          \n\
                    xorl %%ecx, %%ecx           \n\
                    addl %%esp, %%ecx           \n\
                    andl $0xffffe000, %%ecx     \n\
                    "
                    :"=c"(current_pcb)
                    :
                    :"memory", "cc"
        );

        /* store the return value the ebp, esp and parent pcb pointer in the process' pcb */
        process_pcb->ra = ra;
        process_pcb->ebp = ebp;
        process_pcb->esp = esp_ex;
        process_pcb->pp  = current_pcb;
        process_pcb->isBaseTerm = 0x01;
        memcpy(process_pcb->pname, title, 48);

        /* setup the terminal for the process */
        process_pcb->term = term;
        process_pcb->curdir = 0;
        schedule[term] = process_pcb;
        gterminals[term].executed = 1;
        gterminals[term].disabled = 0;

        process_pcb->queue_head = NULL;
        process_pcb->queue_tail = NULL;

        process_term = term;

        /* setup stdin and stdout for the process */
        setup_stdinout();

        gui_element_t * t = alloc_gui_element();
        terminal_element_initiliazer(t, term);
        gterminals[term].background = COLOR_EXPAND_WHITE;
        gterminals[term].text = COLOR_EXPAND_BLACK;

        // add to the end of root
        gui_root.num_children++;
        append_element(&(gui_root.children), &(gui_root.children_tail), t);

        process_pcb->gui_term = t;
        process_pcb->queue_head = NULL;
        process_pcb->queue_tail = NULL;

        // incrememnt num of root children
        // allocate root element

        /* save all registers in the pcb */
        asm volatile("                      \n\
                      movl %%eax, 20(%%eax) \n\
                      movl %%ebx, 24(%%eax) \n\
                      movl %%ecx, 28(%%eax) \n\
                      movl %%edx, 32(%%eax) \n\
                      movl %%esi, 36(%%eax) \n\
                      movl %%edi, 40(%%eax) \n\
                      pushf                 \n\
                      popl %%ebx            \n\
                      movl %%ebx, 44(%%eax) \n\
                      "
                      :
                      :"a"(process_pcb)
                      :"memory"
        );

        /* halt status return */
        int ret;
        /* IRET into the user level program and start executing code
         * first cli to disable interrupts during critical section
         * push user data segment, user esp, user code segment, user
         * eip. Push and pop the flags into %eax and then or with 0x0200
         * to enable interrupts in the user, finally iret to start execution */
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
                      l:                    \n\
                      "
                      :"=a"(ret)
                      :"a"(USER_DS), "b"(esp), "c"(USER_CS), "d"(eip)
                      :"memory", "cc"
        );
        return ret;
    }

    /* not an executable return -1 */
    else{
        free_pid(pid);
        return -1;
    }
}

/* setup_stdinout
 * DESCRIPTION: open stdin and stdout for process and assign them file descriptor 0 and 1
 * OUTPUTS: None
 * RETURN VALUE: None
 * INPUTS: None
 * SIDE EFFECTS: None */
void setup_stdinout() {

  // Initialize stdin
  file_t* rel_file;
  rel_file = &(process_pcb->file[0]);
  rel_file->file_op = &stdin_table;
  rel_file->flags = 1;

  // Initialize stdout
  rel_file = &(process_pcb->file[1]);
  rel_file->file_op = &stdout_table;
  rel_file->flags = 1;

  /* all other files absent */
  unsigned i;
  for (i = 2; i < MAX_FD_LENGTH; i++) {
    process_pcb->file[i].flags = 0;
  }

}

/* halt
 * DESCRIPTION: System call that halts currently executing process and
 *              returns a return value given in status. Reinitialize
 *              paging and tss to make sure parent process executes normally
 *              then, jump to parents' execute call
 * INPUTS: uint8_t status - status of the halt (0 normal), (256 exception), (other abnormal)
 * OUTPUTS: None
 * SIDE EFFECTS: stops currently executing process
 * RETURN VALUE: never returns */
void kill_terminal(int term){

  unsigned long flags;
  cli_and_save(flags);

  pcb_t * cur = schedule[term];
  if(cur == NULL) return;
  while(!(cur->isBaseTerm)){

    pcb_t * temp = cur;
    cur = cur->pp;
    free_pid(temp->pid);

  }
  if(cur->isBaseTerm){ // switch to terminal 1

    gterminals[cur->term].executed = 0;
    gterminals[cur->term].read_mode = 0;
    gterminals[cur->term].r1_mode = 0;
    gterminals[cur->term].buffer_counter = 0;
    /* decrement process count */
    free_pid(cur->pid);

    num_active_terminals--;

    gui_root.num_children--;

    reset_graphic_term(cur->term);

    remove_user_elements(cur->term + 1);

    destroy_terminal(cur->term + 1);

    free_terminal(cur->term);
    //remove_element(&(gui_root.children), &(gui_root.children_tail), current_pcb->term + 1);

    cur->gui_term = NULL;

    if(gui_root.children_tail == NULL){
      curterm = -1;
      process_pcb = NULL;
    }
    else{
      curterm = gui_root.children_tail->id - 1;
      switch_graphic_terminal(curterm);
    }

    enable_irq(MOUSE_IRQ);

    context_switch(NULL);
  }

  restore_flags(flags);

  return;
}

/* halt
 * DESCRIPTION: System call that halts currently executing process and
 *              returns a return value given in status. Reinitialize
 *              paging and tss to make sure parent process executes normally
 *              then, jump to parents' execute call
 * INPUTS: uint8_t status - status of the halt (0 normal), (256 exception), (other abnormal)
 * OUTPUTS: None
 * SIDE EFFECTS: stops currently executing process
 * RETURN VALUE: never returns */
int32_t halt(uint8_t status){

   if(curterm == -1)
    return -1;

    /* return handling for exceptions or other cases */
    int retval;
    if(status == 0){ /* normal halt */
      retval = 0;
    }
    else if(status == 1){ /* exception */
      retval = 256;
    }
    else{ /* something else */
      retval = 1;
    }

    unsigned long flags;
    cli_and_save(flags);

    /* get the currently executing process PCB pointer
     * by applying the bitmask 0xffffe000 to get to the
     * top of the PCB block of the process */
    pcb_t * current_pcb;

    asm volatile("                          \n\
                xorl %%ecx, %%ecx           \n\
                addl %%esp, %%ecx           \n\
                andl $0xffffe000, %%ecx     \n\
                "
                :"=c"(current_pcb)
                :
                :"memory"
    );
    /* if this is the base shell, then tell user that it cannot be
     * exited by printing a message and re-execute shell */
    // if(current_pcb->pid == 0){
    //     gterminals[current_pcb->term].executed = 0;
    //     gterminals[current_pcb->term].read_mode = 0;
    //     /* decrement process count */
    //     free_pid(current_pcb->pid);

    //     num_active_terminals--;

    //     gui_root.num_children--;

    //     reset_graphic_term(current_pcb->term);

    //     destroy_terminal(current_pcb->term + 1);

    //     if(gui_root.children_tail == NULL)
    //       curterm = -1;
    //     else{
    //       curterm = gui_root.children_tail->id - 1;
    //       switch_graphic_terminal(curterm);
    //     }

    //     context_switch(NULL);
      // graphic_printf("Cannot exit base shell\n");
      // free_pid(0);
      // /* make process count zero again */
      // pcount = 0;

      // num_active_terminals--;
      // gterminals[current_pcb->term].scheduled = 0;
      // reset_graphic_term(current_pcb->term);

      // gui_element_t * t = gui_root.children_tail;
      // t->render = RENDER_OFF;
      // t->type |= TYPE_INVISIBLE;
      // if(t->prev == NULL)
      //   curterm = -1;
      // else{
      //   curterm = t->prev->id - 1;
      //   switch_graphic_terminal(curterm);
      // }
      // execute("shell");
      if(current_pcb->isBaseTerm){ // switch to terminal 1
        gterminals[current_pcb->term].executed = 0;
        gterminals[current_pcb->term].read_mode = 0;
        gterminals[current_pcb->term].r1_mode = 0;
        gterminals[current_pcb->term].buffer_counter = 0;
        /* decrement process count */
        free_pid(current_pcb->pid);

        num_active_terminals--;

        gui_root.num_children--;

        reset_graphic_term(current_pcb->term);

        remove_user_elements(current_pcb->term + 1);

        // remove_user_elements();
        destroy_terminal(current_pcb->term + 1);

        free_terminal(current_pcb->term);
        //remove_element(&(gui_root.children), &(gui_root.children_tail), current_pcb->term + 1);

        current_pcb->gui_term = NULL;

        if(gui_root.children_tail == NULL){
          curterm = -1;
          process_pcb = NULL;
        }
        else{
          curterm = gui_root.children_tail->id - 1;
          switch_graphic_terminal(curterm);
        }

        context_switch(NULL);
      }
      else{

        /* close any opened file */
        int i;
        for(i = 0; i < MAX_FILE; i++){
          if(current_pcb->file[i].flags == 1){

            current_pcb->file[i].file_op->close();
            current_pcb->file[i].flags = 0;

          }
        }

        remove_user_elements(current_pcb->term + 1);
        gterminals[current_pcb->term].disabled = 0;
        gterminals[current_pcb->term].r1_mode = 0;
        gterminals[current_pcb->term].background = COLOR_EXPAND_WHITE;
        gterminals[current_pcb->term].text = COLOR_EXPAND_BLACK;
        process_pcb = current_pcb->pp;
        set_graphic_terminal_title(process_pcb->term, process_pcb->pname);
        schedule[process_pcb->term] = process_pcb;
      }
      teardown_process(current_pcb);
      /* decrement process count */
      free_pid(current_pcb->pid);

    /* restore the ebp, esp, ra values using the current_pcb */
    asm volatile("                          \n\
                movl 12(%%ecx), %%ebp       \n\
                movl 8(%%ecx), %%ebx        \n\
                movl 16(%%ecx), %%edx       \n\
                movl %%edx, %%esp           \n\
                jmp h                       \n\
                "
                :
                :"c"(current_pcb), "a"(retval)
                :"memory"
    );

    /* never reached */
    return 0;
}

/* open
 * DESCRIPTION: opens the file given by the input filename and returns the file descriptor
 *              for the opened file back to the process
 * INPUTS: uint8_t * filename - filename of file to open
 * OUTPUTS: None
 * SIDE EFFECTS: opens file (or error)
 * RETURN VALUE: int32_t 0 for success, -1 for failure */

/* I THINK THERE IS AN ERROR IN THIS FUNCTION REMIND XANNY */
int32_t open(const uint8_t * filename) {

  if (filename == NULL) {
    return -1;
  }

  char filenamec[32];
  strncpy((int8_t *)filenamec, (int8_t *)filename, 32);

  uint32_t length = strlen(filenamec);
  if(length != 0 && (filenamec[length - 1] == '/' || filenamec[length - 1] == '\n')) {
    filenamec[length - 1] = '\0';
  }

  file_t* rel_file = NULL;
  unsigned i;

  // Try to get an Empty space, if not found return -1
  for (i = 2; i < 8; i++) {
    /* if file flags is 0 means space is free */
    if (process_pcb->file[i].flags == 0) {
      rel_file = &(process_pcb->file[i]);
      rel_file->flags = 1;
      break;
    }
    if (i == 7) {
      return -1;
    }
  }

  dentry_ext_t dentry;

  if(strncmp(filenamec, "rtc", strlen(filenamec)) == 0){
    find_dentry_ext("rtc", &dentry, 3);
  }
  else if(strncmp(filenamec, "gui", strlen(filenamec)) == 0){
    find_dentry_ext("gui", &dentry, 3);
  }
  else if(strncmp(filenamec, "sb16", strlen(filenamec)) == 0){
    find_dentry_ext("sb16", &dentry, 3);
  }
  else{

    // Find the file/directory
    //int32_t ret_val = read_dentry_by_name(filename, &dentry);
    char fname[32];
    reset_buffer(fname);
    int inode = parse_path(filenamec, process_pcb->curdir, fname);

    if(inode == -1) return -1;

    if (udp_dir.inode != 0) {
      if (udp_dir.inode == inode) {
        rel_fd = i;
        rel_file->file_op = &net_table;
        rel_file->inode = 0;
        rel_file->fileposition = 0;
        strcpy(open_filename, filenamec);
        int net_retval = rel_file->file_op->open();
        if (net_retval == -1) {
          rel_file->flags = 0;
          return -1;
        } else {
          return i;
        }
      }
    }

    int32_t ret_val = find_dentry_ext(fname, &dentry, inode);

    // If not found return -1 for fail
    if (ret_val == -1) {
      return -1;
    }

  }

  // Just to make sure code won't segfault
  if (rel_file == NULL) {
    return -1;
  }

  rel_fd = i;

  // filetype 2 is rtc
  if (dentry.filetype == 2) {
    if(strncmp(dentry.filename, "rtc", strlen(filenamec)) == 0){
      rel_file->file_op = &rtc_table;
      rel_file->inode = NUM_INODES;
      rel_file->fileposition = 0;
      rel_file->file_op->open();
    }
    else if(strncmp(dentry.filename, "gui", strlen(filenamec)) == 0){
      rel_file->file_op = &gui_table;
      rel_file->inode = NUM_INODES;
      rel_file->fileposition = 0;
      rel_file->file_op->open();
    }
    else if(strncmp(dentry.filename, "sb16", strlen(filenamec)) == 0){
      rel_file->file_op = &sb16_table;
      rel_file->inode = NUM_INODES;
      rel_file->fileposition = 0;
      rel_file->file_op->open();
    }
  // filtype 0 is a directory
  } else if (dentry.filetype == 0) {
    rel_file->file_op = &dir_table;
    rel_file->fileposition = 0;
    rel_file->inode = dentry.inode;

  // filetype 1 is a regular file
  } else if (dentry.filetype == 1) {
    rel_file->fileposition = 0;
    rel_file->inode = dentry.inode;
    rel_file->file_op = &file_table;
  }

  return i;

}

/* close
 * DESCRIPTION: close a file given by the file descriptor
 * INPUTS: int32_t fd - file descriptor of which file to close
 * OUTPUTS: None
 * SIDE EFFECTS: closes file
 * RETURN VALUE: int32_t 0 for success, -1 for failure */
int32_t close(int32_t fd) {

    // Check if fd is valid
    if (fd < 2 || fd >= MAX_FD_LENGTH) {
      return -1;
    }

    // check if rel_file is preset
    if (process_pcb->file[fd].flags == 0) {
      return -1;
    }

    // Clear up to add a new file
    rel_fd = fd;
    int32_t retval = process_pcb->file[fd].file_op->close();
    process_pcb->file[fd].flags = 0;
    process_pcb->file[fd].fileposition = 0;
    process_pcb->file[fd].file_op = NULL;

    // return 0 for success
    return 0;

}

/*
 * getargs
 * DESCRIPTION: Syscall to implement get arguments for user level programs
 * INPUTS: buf -- buffer to put arguments into
 *         nbytes -- number of bytes to copy into buf
 * OUTPUTS: None
 * SIDE EFFECTS: None
 * RETURN VALUE: 0 for success, -1 for failure
 */
int32_t getargs(uint8_t * buf, int32_t nbytes){

  // If arguments are empty just return
  if (g_args[0] == '\0') {
    return -1;
  }

  // Checks to see if all arguments can be copied wrt nbytes length
  if (g_arg_counter + 1 > nbytes) {
    return -1;
  }

  // copy bytes to buffer
  memcpy(buf, g_args, g_arg_counter + 1);

  // return for success
  return 0;
}

/*
 * vidmap
 * DESCRIPTION: Syscall to give user programs access to video memory
 * INPUTS: screen_start -- memory location to put start of video memory in
 * OUTPUTS: None
 * SIDE EFFECTS: Possibly gives user access to video memory
 * RETURN VALUE: 0 for success, -1 for failure
 */
int32_t vidmap(uint8_t ** screen_start) {

    // check if pointer is valid and inside user memory space
    // return -1 for illegal pointer
    if (screen_start == NULL || (uint8_t*) screen_start < (uint8_t*) PROCESS_START
        || (uint8_t*) screen_start >= (uint8_t*) PROCESS_START + FOURMB) {

      return -1;
    }

    unsigned long flags;
    cli_and_save(flags);

    int i;
    int term = process_pcb->term;
    uint32_t terminal_start = 0xFC000000 + TERMINAL_0_START_OFFSET + term * TERMINAL_SIZE;

    for(i = 0; i < 8; i++){

      page_table_vidmem[i].p = 0x1;
      page_table_vidmem[i].base_address = FOURKB_ADDRESS(terminal_start);
      terminal_start += 4*1024;

    }

    page_dir[VIDMEM_PAGE].base_address = FOURKB_ADDRESS((uint32_t)page_table_vidmem);

    set_cr((void *)page_dir);

    // give user access to start of video memory
    *screen_start = start_of_vidmem;

    restore_flags(flags);
    return 0;
}

/*
 * set_handler
 * DESCRIPTION: not implemented
 * INPUTS: Ignore
 * OUTPUTS: None
 * RETURN VALUE: -1, always fails
 */
int32_t set_handler(int32_t signum, void * handler_address) {
    if (signum < 0 || signum >= SIGNAL_NUM || handler_address == NULL) {
      return -1;
    }

    process_pcb->sigs.sig_ptr[signum] = handler_address;
    return 0;
}

/*
 * sigreturn
 * DESCRIPTION: not implemented
 * INPUTS: Ignore
 * OUTPUTS: None
 * RETURN VALUE: -1, always fails
 */
int32_t sigreturn(void) {

    // copy over hardware context from user stack

    ptregs_t temp_ptregs;
    uint32_t esp = process_pcb->sig_hw_esp;
    memcpy(&temp_ptregs, (uint8_t*) esp, sizeof(ptregs_t));

    // if we segfault or divide by zero we still kill the program
    if (temp_ptregs.sig_num == 0 || temp_ptregs.sig_num == 1) {
      halt(1);
    }

    execute_signals();

    // call sigreturn_helper which Reinitializes hardware_context and irets
    sigreturn_helper(temp_ptregs);

    // should never reach
    return -1;
}

/*
 * guireturn
 * DESCRIPTION: not implemented
 * INPUTS: Ignore
 * OUTPUTS: None
 * RETURN VALUE: -1, always fails
 */
int32_t guireturn(void) {

    // copy over hardware context from user stack
    ptregs_t temp_ptregs;
    uint32_t esp = process_pcb->gui_hw_esp;
    memcpy(&temp_ptregs, (uint8_t*) esp, sizeof(ptregs_t));

    int id = temp_ptregs.sig_num;

    gui_element_t * t = find_element_by_id(process_pcb->gui_term->children, id);

    if(t == NULL){
      return -1;
    }

    t->type &= ~(TYPE_LOADED);

    execute_gui_handlers();

    // call sigreturn_helper which Reinitializes hardware_context and irets
    cli();
    guireturn_helper(temp_ptregs);

    // should never reach
    return -1;
}

int32_t chdir(uint8_t * buf){

  char name[32];
  int dir = parse_path((char *)buf, process_pcb->curdir, name);

  if(dir == -1) return -1;

  if(name[0] == ' '){

    process_pcb->pp->curdir = dir;
    return 0;

  }
  else{

    dentry_ext_t den;
    int ret = find_dentry_ext(name, &den, dir);
    if(ret == -1) return -1;
    if(den.filetype != 0) return -1;
    process_pcb->pp->curdir = den.inode;
    return 0;

  }


}

int32_t rmdir(uint8_t * buf){

  char name[32];
  int dir = parse_path((char *)buf, process_pcb->curdir, name);

  if(dir == -1) return -1;

  if(name[0] == ' '){

    return delete_dir(dir);

  }
  else{

    dentry_ext_t den;
    int ret = find_dentry_ext(name, &den, dir);
    if(ret == -1) return -1;
    if(den.filetype != 0) return -1;

    return delete_dir(den.inode);

  }

}

int32_t mkdir(uint8_t * buf){

  char name[32];
  int dir = parse_path((char *)buf, process_pcb->curdir, name);
  if(dir == -1) return -1;
  if(name[0] == ' ') return -1;
  return make_directory(name, dir);

}

int32_t create(uint8_t * buf){

  char name[32];
  int dir = parse_path((char *)buf, process_pcb->curdir, name);
  if(dir == -1) return -1;
  if(name[0] == ' ') return -1;
  return make_empty_file_ext(name, dir);

}

int32_t unlink(uint8_t * buf){

  char name[32];
  int dir = parse_path((char *)buf, process_pcb->curdir, name);

  if(dir == -1) return -1;

  if(name[0] == ' '){

    return -1;

  }
  else{

    dentry_ext_t den;
    int ret = find_dentry_ext(name, &den, dir);
    if(ret == -1) return -1;
    if(den.filetype != 1) return -1;

    return delete_file_ext(den.inode, dir);

  }

}

int32_t ioctl(int32_t fd, unsigned long cmd, unsigned long arg){

  // Check if fd is invalid and if nbytes is negative
  if (fd < 0 || fd >= MAX_FD_LENGTH) {
    return -1;
  }

  // Get the relevant file and check if its present
  file_t* rel_file = &(process_pcb->file[fd]);
  if (rel_file->flags == 0) {
    return -1;
  }

  // return no. of bytes written
  rel_fd = fd;
  return rel_file->file_op->ioctl(cmd, arg);

}
