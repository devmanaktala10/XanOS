#include "page.h"

/* init_paging
 * DESCRIPTION: Initialize all the page directory entires and the page table 1
 *              entries to empty. Set the Kernel page and the 4Kb video memory
 *              page to present. Call the function that sets the apprioriate registers
 * INPUTS: None
 * OUTPUTS: None
 * RETURN VALUE: None
 * SIDE EFFECTS: Paging gets turned on
 * */
void init_paging(){

    int i;

    /* initialize the page directory to empty entries */
    for(i = 0; i < PAGE_DIR_LEN; i++){
        page_dir[i].p  = 0x0; /* not present */
        page_dir[i].rw = 0x1; /* read and write access */
        page_dir[i].us = 0x0; /* only kernel access */
        page_dir[i].wt = 0x0;
        page_dir[i].cd = 0x0;
        page_dir[i].ac = 0x0;
        page_dir[i].d  = 0x0;
        page_dir[i].ps = 0x0; /* 4kb pages */
        page_dir[i].g  = 0x0;
        page_dir[i].avail = 0x0;
        page_dir[i].base_address = 0x00; /* page table base address */
    }

    /* initialize the page table 1 to empty entries */
    for(i = 0; i < PAGE_TABLE_LEN; i++){
        page_table_1[i].p  = 0x0; /* not present */
        page_table_1[i].rw = 0x1; /* read and write access */
        page_table_1[i].us = 0x0; /* only kernel access */
        page_table_1[i].wt = 0x0;
        page_table_1[i].cd = 0x0;
        page_table_1[i].ac = 0x0;
        page_table_1[i].d  = 0x0;
        page_table_1[i].ps = 0x0;
        page_table_1[i].g  = 0x0;
        page_table_1[i].avail = 0x0;
        page_table_1[i].base_address = 0x00001 * i; /* 4kb aligned base address of the pages */
    }

    /* the first page directry (0-4MB) entry is present */
    page_dir[0].p = 0x1;
    /* 32 bit 4kb aligned address to page table 1 */
    uint32_t pointer = (uint32_t) page_table_1;
    /* set the base address of the page table */
    page_dir[0].base_address = FOURKB_ADDRESS(pointer);
     /* video memory page exists */
    page_table_1[FOURKB_ADDRESS(VIDEO)].p  = 0x1;

    /* kernel directory entry present */
    page_dir[KERNEL_PAGE].g = 0x1;
    /* kernel directory entry present */
    page_dir[KERNEL_PAGE].p = 0x1;
    /* kernel page is one 4MB page */
    page_dir[KERNEL_PAGE].ps = 0x1; /* 4 MB page mode */
    /* Base address of 4MB physical memory */
    page_dir[KERNEL_PAGE].base_address = FOURKB_ADDRESS(FOURMB);


    /* user page dir process space 4MB */
    page_dir[PROCESS_PAGE].p = 0x0;
    page_dir[PROCESS_PAGE].ps = 0x1;
    page_dir[PROCESS_PAGE].us = 0x1; /* user page */


    // set up vidmap
    page_dir[PROCESS_PAGE + 1].p = 0x1;  // make present
    page_dir[PROCESS_PAGE + 1].ps = 0x0; // 4KB page mode
    page_dir[PROCESS_PAGE + 1].us = 0x1; /* user page */

    // Map page table from 0 to 4MB
    for(i = 0; i < PAGE_TABLE_LEN; i++) {
        page_table_vidmem[i].p  = 0x0; /* not present */
        page_table_vidmem[i].rw = 0x1; /* read and write access */
        page_table_vidmem[i].us = 0x1; /* user access */
        page_table_vidmem[i].wt = 0x0;
        page_table_vidmem[i].cd = 0x0;
        page_table_vidmem[i].ac = 0x0;
        page_table_vidmem[i].d  = 0x0;
        page_table_vidmem[i].ps = 0x0;
        page_table_vidmem[i].g  = 0x0;
        page_table_vidmem[i].avail = 0x0;
        page_table_vidmem[i].base_address = 0x00001 * i; /* 4kb aligned base address of the pages */
    }

    // change base address of page_dir to point to page table
    // pointer = (uint32_t) page_table_vidmem;
    page_dir[PROCESS_PAGE + 1].base_address = FOURKB_ADDRESS(pointer);

    // setup networking pages
    page_dir[NETWORK_PAGE].p = 0x1;  // make present
    page_dir[NETWORK_PAGE].ps = 0x1; // 4MB page mode
    page_dir[NETWORK_PAGE].us = 0x0; /* kernel page */
    page_dir[NETWORK_PAGE].cd = 1;
    page_dir[NETWORK_PAGE].base_address = FOURKB_ADDRESS(NETWORK_PAGE_ADDR);

    page_dir[NETWORK_MEM_PAGE].p = 0x1;  // make present
    page_dir[NETWORK_MEM_PAGE].ps = 0x1; // 4MB page mode
    page_dir[NETWORK_MEM_PAGE].us = 0x0; /* Kernel page */
    page_dir[NETWORK_MEM_PAGE].base_address = FOURKB_ADDRESS(NETWORK_MEM_PAGE_ADDR);

    page_dir[SOUND_PAGE].p = 0x1;
    page_dir[SOUND_PAGE].ps = 0x1;
    page_dir[SOUND_PAGE].us = 0x0;
    page_dir[SOUND_PAGE].base_address = FOURKB_ADDRESS(SOUND_PAGE*4*1024*1024);
    page_dir[SOUND_PAGE].cd = 0x1;

    // page_table_vidmem[FOURKB_ADDRESS(VIDEO)].p = 0x1; // video memory page is present

    // Initialize start of video memory pointer returned to user level programs
    start_of_vidmem = (uint8_t*) (VIDMEM_START);

    /* Set the paging to enable processor paging */
    set_cr((void *) page_dir);

}


void setup_video_mem(uint32_t BAR0){

    unsigned long flags;
    cli_and_save(flags);

    // uint32_t test = FOURMB_ADDRESS(BAR0);

    page_dir[1008].p = 0x1;
    page_dir[1008].ps = 0x1;
    page_dir[1008].us = 0x0;
    page_dir[1008].base_address = FOURKB_ADDRESS(BAR0);

    page_dir[1009].p = 0x1;
    page_dir[1009].ps = 0x1;
    page_dir[1009].us = 0x0;
    page_dir[1009].base_address = FOURKB_ADDRESS(BAR0 + FOURMB);

    page_dir[1010].p = 0x1;
    page_dir[1010].ps = 0x1;
    page_dir[1010].us = 0x0;
    page_dir[1010].base_address = FOURKB_ADDRESS(BAR0 + 2*FOURMB);

    page_dir[1011].p = 0x1;
    page_dir[1011].ps = 0x1;
    page_dir[1011].us = 0x0;
    page_dir[1011].base_address = FOURKB_ADDRESS(BAR0 + 3*FOURMB);

    set_cr((void *) page_dir);

    restore_flags(flags);

}
