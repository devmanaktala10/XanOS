#include "process.h"

void init_signals() {
    default_signals.sig_ptr[0] = &kill_process;
    default_signals.sig_ptr[1] = &kill_process;
    default_signals.sig_ptr[2] = &kill_process;
    default_signals.sig_ptr[3] = &ignore_sig;
    default_signals.sig_ptr[4] = &ignore_sig;
    default_signals.bitvector = 0x0;
}

void setup_process(int pid, uint32_t inode_num){

    uint32_t phy_addr = EIGHTMB + FOURMB + FOURMB * pid;

    /* process page at 128 MB is pointing at physical */
    page_dir[PROCESS_PAGE].base_address = FOURKB_ADDRESS(phy_addr);
    /* process page at is present*/
    page_dir[PROCESS_PAGE].p = 0x1;
    /* page directory */
    set_cr((void *) page_dir);

    /* start loading program into memory */
    uint32_t length = read_length_ext(inode_num);
    char filebuf[1024];
    uint32_t offset = 0;

    while(length > 1024){
        read_data_ext(inode_num, offset, (uint8_t*) filebuf, 1024);
        memcpy((void *)(PROCESS_START + offset), (uint8_t*) filebuf, 1024);
        offset += 1024;
        length -= 1024;
    }

    read_data_ext(inode_num, offset, (uint8_t*) filebuf, length);
    memcpy((void *)(PROCESS_START + offset), filebuf, length);

    /* program is now copied into memory, make its kernel stack */
    tss.ss0 = KERNEL_DS;
    tss.esp0 = EIGHTMB - pid * (KERNEL_STACK_SIZE);
}

void teardown_process(pcb_t * process_pcb){

    int parent_pid = process_pcb->pp->pid;
    uint32_t phy_addr = EIGHTMB + FOURMB + FOURMB * parent_pid;
    page_dir[PROCESS_PAGE].base_address = FOURKB_ADDRESS(phy_addr);
    tss.esp0 = EIGHTMB - parent_pid * (KERNEL_STACK_SIZE);
    set_cr((void *) page_dir);
    
}

void sched_process(){
    /* find next process to schedule round-robin*/

    if(num_active_terminals == 0){
        // int i;
        // for(i = 0; i < MAX_SCHEDULE; i++){

        //     if(gterminals[i].executed == 1 && gterminals[i].scheduled == 1){
        //         num_active_terminals++;
        //         context_switch(schedule[i]);
        //         asm volatile("iret");
        //     }

        // }

        empty_process();

    }

    int i;
    pcb_t * next_pcb = process_pcb;
    int ct = process_term;

    for(i = 0; i < MAX_SCHEDULE + 1; i++){
        ct++;
        ct = ct % MAX_SCHEDULE;

        if(gterminals[ct].executed && ct != process_term && !(ct != curterm && gterminals[ct].read_mode == 1)){
            next_pcb = schedule[ct];
            break;
        }
    }

    /* if no other schedulable processes */
    if(process_term == ct) return;

    context_switch(next_pcb);
}

void context_switch(pcb_t * next_pcb){

    if(next_pcb == NULL){
        
        asm volatile("                         \n\
            movl %%esp, 0(%%eax)      \n\
            movl %%ebp, 4(%%eax)      \n\
            "
            :
            :"a"(&sched_swaps[process_term])
            :"memory"
        );

        process_term = -1;
        process_pcb = NULL;
        empty_process();

    }

    /* setup paging */
    uint32_t phy_addr = EIGHTMB + FOURMB + FOURMB * next_pcb->pid;
    /* process page at 128 MB is pointing at physical */
    page_dir[PROCESS_PAGE].base_address = FOURKB_ADDRESS(phy_addr);
    /* process page at is present*/
    page_dir[PROCESS_PAGE].p = 0x1;
    /* page directory */
    set_cr((void *) page_dir);

    /* setup tss */
    tss.esp0 = EIGHTMB - next_pcb->pid * (KERNEL_STACK_SIZE);

    int i;
    int term = next_pcb->term;
    uint32_t terminal_start = 0xFC000000 + TERMINAL_0_START_OFFSET + term * TERMINAL_SIZE;

    for(i = 0; i < 8; i++){

      page_table_vidmem[i].p = 0x1;
      page_table_vidmem[i].base_address = FOURKB_ADDRESS(terminal_start);
      terminal_start += 4*1024;

    } 

    page_dir[VIDMEM_PAGE].base_address = FOURKB_ADDRESS((uint32_t)page_table_vidmem);

    set_cr((void *)page_dir);

    // if(next_pcb->term == curterm){
    //     // map video memory to actual DMA
    //     page_table_1[FOURKB_ADDRESS(VIDEO)].base_address = FOURKB_ADDRESS(VIDEO);
    //     page_table_vidmem[FOURKB_ADDRESS(VIDEO)].base_address = FOURKB_ADDRESS(VIDEO);
    //     set_cr((void *) page_dir);
    // }
    // else{
    //     // map video memory to swap space
    //     uint32_t phy_addr = FOURKB * (next_pcb->term + 1);
    //     page_table_1[FOURKB_ADDRESS(VIDEO)].base_address = FOURKB_ADDRESS(phy_addr);
    //     page_table_vidmem[FOURKB_ADDRESS(VIDEO)].base_address = FOURKB_ADDRESS(phy_addr);
    //     set_cr((void *) page_dir);
    // }

    asm volatile("                         \n\
                movl %%esp, 0(%%eax)      \n\
                movl %%ebp, 4(%%eax)      \n\
                "
                :
                :"a"(&sched_swaps[process_term])
                :"memory"
    );

    // int prev_term = process_term;
    process_term = next_pcb->term;
    process_pcb = next_pcb; 

    asm volatile("                         \n\
                movl 4(%%eax), %%ebp      \n\
                movl 0(%%eax), %%esp      \n\
                "
                :
                :"a"(&sched_swaps[next_pcb->term])
                :"memory"
    );

}

int32_t stdin_open(){
    return -1;
}

int32_t stdin_close(){
    return -1;
}

int32_t stdin_write(){
    return -1;
}

int32_t stdin_ioctl(unsigned long cmd, unsigned long arg){
    return -1;
}

int32_t stdout_open(){
    return -1;
}

int32_t stdout_close(){
    return -1;
}

int32_t stdout_read(){
    return -1;
}

int32_t stdout_ioctl(unsigned long cmd, unsigned long arg){
    return -1;
}

void reset_pids(){

    pid[0] = 0x00;
    pid[1] = 0x00;
    pid[2] = 0x00;

}

void set_pid(uint32_t index, int val){

    uint32_t i = index/8;
    uint32_t shift = index%8;
    if(val == 1)
        pid[i] = pid[i] | (0x01 << shift);
    else
        pid[i] = pid[i] & (~(0x01 << shift));

}

int get_pid(uint32_t index){

    uint32_t i = index/8;
    uint32_t shift = index%8;
    if(pid[i] & (0x01 << shift)) return 1;
    else return 0;

}

int find_free_pid(){

    uint32_t i = 0;
    unsigned long flags;
    cli_and_save(flags);

    for(i = 0; i <= MAX_PID; i++){

        if(get_pid(i) == 0){
            set_pid(i, 1);
            restore_flags(flags);
            return i;
        }

    }
    
    return -1;
    restore_flags(flags);

}

void free_pid(uint32_t index){

    unsigned long flags;
    cli_and_save(flags);

    set_pid(index, 0);

    restore_flags(flags);
}

void empty_process(){

    sti();
    asm volatile (".1: hlt; jmp .1;");

}
