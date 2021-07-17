#ifndef _PAGE_H
#define _PAGE_H

#include "../types.h"
#include "../x86_desc.h"
#include "../lib.h"

#define PAGE_DIR_LEN 1024
#define PAGE_TABLE_LEN 1024
#define FOURKB 4096
#define KERNEL_PAGE 1
#define FOURMB 0x0400000
#define EIGHTMB 0x0800000
#define PROCESS_ADDRESS 0x08000000
#define PROCESS_START 0x08048000
#define VIDMEM_START (PROCESS_ADDRESS + FOURMB)
#define PROCESS_PAGE (PROCESS_ADDRESS/FOURMB)
#define VIDMEM_PAGE PROCESS_PAGE + 1
#define KERNEL_STACK_SIZE (FOURKB * 2)
#define NETWORK_PAGE          500
#define NETWORK_PAGE_ADDR     (NETWORK_PAGE * FOURMB)
#define NETWORK_MEM_PAGE      40
#define NETWORK_MEM_PAGE_ADDR (NETWORK_MEM_PAGE * FOURMB)
#define SOUND_PAGE 3 
#define SOUND_PAGE_2 4
#define SOUND_PAGE_3 5
#define SOUND_PAGE_4 6
#define SOUND_PAGE_5 7
#define SOUND_PAGE_6 8 
/* Macro to convert 4KB aligned 32 bit address to a 20 bit
 * address in order to store in the page directory and page tables.
 * This is accomplished by right shifting the address by 12 bits since
 * the last 12 bits are 0 for a 4KB aligned address */
#define FOURKB_ADDRESS(n) ((n) >> 12)

#define FOURMB_ADDRESS(n) ((n) >> 24)

/* Structure for Page Directory Entries and Page Table Entries */
typedef struct page_entry{
    uint32_t p              :1;     /* Present          */
    uint32_t rw             :1;     /* Read/Write       */
    uint32_t us             :1;     /* User/Supervisor  */
    uint32_t wt             :1;     /* Write-Through    */
    uint32_t cd             :1;     /* Cache Disabled   */
    uint32_t ac             :1;     /* Accessed         */
    uint32_t d              :1;     /* Dirty            */
    uint32_t ps             :1;     /* Page-Size/PAT    */
    uint32_t g              :1;     /* Global-Page      */
    uint32_t avail          :3;     /* Avail bits       */
    uint32_t base_address   :20;    /* Base address     */
    /* Note: For a 4MB page, the base address is only 10 bits long */
}__attribute__ ((packed)) page_entry_t; /* Pack structure into 32 bits */

uint8_t* start_of_vidmem;

/* kernel page directory: align to 4KB */
page_entry_t page_dir[PAGE_DIR_LEN] __attribute__((aligned (FOURKB)));

/* Page table for first page directory entry: align to 4KB */
page_entry_t page_table_1[PAGE_TABLE_LEN] __attribute__((aligned (FOURKB)));

/* Page table for the vidmap to work */
page_entry_t page_table_vidmem[PAGE_TABLE_LEN] __attribute__((aligned (FOURKB)));

/* swap terminal pages */


/* Paging Functions */
void init_paging();
extern void set_cr(void * cr3_addr);
void setup_video_mem(uint32_t BAR0);

#endif
