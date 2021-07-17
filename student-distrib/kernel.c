/* kernel.c - the C part of the kernel
 * vim:ts=4 noexpandtab
 */

#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "debug.h"
#include "tests.h"
#include "./drivers/i8259.h"
#include "./drivers/pit.h"
#include "./drivers/keyboard.h"
#include "./drivers/rtc.h"
#include "./drivers/terminal.h"
#include "./processes/process.h"
#include "./filesystem/filesystem.h"
#include "./syscalls/system_call.h"
#include "./exceptions/exception.h"
#include "./interrupts/interrupt.h"
#include "./paging/page.h"
#include "./filesystem/ext2.h"
#include "./signals/signals.h"
#include "./drivers/pci.h"
#include "./drivers/cirrus.h"
#include "./drivers/graphic_terminal.h"
#include "./drivers/gui.h"
#include "./drivers/networking.h"
#include "text.h"

#define RUN_TESTS

//typedef void (*Handler)(const void * buf, int32_t nbytes);
//Handler stdout_jumptable[4]={NULL, NULL,write_terminal};


/* Macros. */
/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags, bit)   ((flags) & (1 << (bit)))

/* Check if MAGIC is valid and print the Multiboot information structure
   pointed by ADDR. */
void entry(unsigned long magic, unsigned long addr) {

    multiboot_info_t *mbi;

    /* Clear the screen. */
    clear();

    /* Am I booted by a Multiboot-compliant boot loader? */
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        printf("Invalid magic number: 0x%#x\n", (unsigned)magic);
        return;
    }

    /* Set MBI to the address of the Multiboot information structure. */
    mbi = (multiboot_info_t *) addr;

    /* Print out the flags. */
    printf("flags = 0x%#x\n", (unsigned)mbi->flags);

    /* Are mem_* valid? */
    if (CHECK_FLAG(mbi->flags, 0))
        printf("mem_lower = %uKB, mem_upper = %uKB\n", (unsigned)mbi->mem_lower, (unsigned)mbi->mem_upper);

    /* Is boot_device valid? */
    if (CHECK_FLAG(mbi->flags, 1))
        printf("boot_device = 0x%#x\n", (unsigned)mbi->boot_device);

    /* Is the command line passed? */
    if (CHECK_FLAG(mbi->flags, 2))
        printf("cmdline = %s\n", (char *)mbi->cmdline);

    if (CHECK_FLAG(mbi->flags, 3)) {
        int mod_count = 0;
        int i;
        module_t* mod = (module_t*)mbi->mods_addr;
        init_fs((uint32_t *) mod->mod_start);
        while (mod_count < mbi->mods_count) {
            printf("Module %d loaded at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_start);
            printf("Module %d ends at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_end);
            init_fs((uint32_t *) mod->mod_start);
            printf("First few bytes of module:\n");
            for (i = 0; i < 16; i++) {
                printf("0x%x ", *((char*)(mod->mod_start+i)));
            }
            printf("\n");
            mod_count++;
            mod++;
        }
    }
    /* Bits 4 and 5 are mutually exclusive! */
    if (CHECK_FLAG(mbi->flags, 4) && CHECK_FLAG(mbi->flags, 5)) {
        printf("Both bits 4 and 5 are set.\n");
        return;
    }

    /* Is the section header table of ELF valid? */
    if (CHECK_FLAG(mbi->flags, 5)) {
        elf_section_header_table_t *elf_sec = &(mbi->elf_sec);
        printf("elf_sec: num = %u, size = 0x%#x, addr = 0x%#x, shndx = 0x%#x\n",
                (unsigned)elf_sec->num, (unsigned)elf_sec->size,
                (unsigned)elf_sec->addr, (unsigned)elf_sec->shndx);
    }

    /* Are mmap_* valid? */
    if (CHECK_FLAG(mbi->flags, 6)) {
        memory_map_t *mmap;
        printf("mmap_addr = 0x%#x, mmap_length = 0x%x\n",
                (unsigned)mbi->mmap_addr, (unsigned)mbi->mmap_length);
        for (mmap = (memory_map_t *)mbi->mmap_addr;
                (unsigned long)mmap < mbi->mmap_addr + mbi->mmap_length;
                mmap = (memory_map_t *)((unsigned long)mmap + mmap->size + sizeof (mmap->size)))
            printf("    size = 0x%x, base_addr = 0x%#x%#x\n    type = 0x%x,  length    = 0x%#x%#x\n",
                    (unsigned)mmap->size,
                    (unsigned)mmap->base_addr_high,
                    (unsigned)mmap->base_addr_low,
                    (unsigned)mmap->type,
                    (unsigned)mmap->length_high,
                    (unsigned)mmap->length_low);
    }

    /* Construct an LDT entry in the GDT */
    {
        seg_desc_t the_ldt_desc;
        the_ldt_desc.granularity = 0x0;
        the_ldt_desc.opsize      = 0x1;
        the_ldt_desc.reserved    = 0x0;
        the_ldt_desc.avail       = 0x0;
        the_ldt_desc.present     = 0x1;
        the_ldt_desc.dpl         = 0x0;
        the_ldt_desc.sys         = 0x0;
        the_ldt_desc.type        = 0x2;

        SET_LDT_PARAMS(the_ldt_desc, &ldt, ldt_size);
        ldt_desc_ptr = the_ldt_desc;
        lldt(KERNEL_LDT);
    }

    /* Construct a TSS entry in the GDT */
    {
        seg_desc_t the_tss_desc;
        the_tss_desc.granularity   = 0x0;
        the_tss_desc.opsize        = 0x0;
        the_tss_desc.reserved      = 0x0;
        the_tss_desc.avail         = 0x0;
        the_tss_desc.seg_lim_19_16 = TSS_SIZE & 0x000F0000;
        the_tss_desc.present       = 0x1;
        the_tss_desc.dpl           = 0x0;
        the_tss_desc.sys           = 0x0;
        the_tss_desc.type          = 0x9;
        the_tss_desc.seg_lim_15_00 = TSS_SIZE & 0x0000FFFF;

        SET_TSS_PARAMS(the_tss_desc, &tss, tss_size);

        tss_desc_ptr = the_tss_desc;

        tss.ldt_segment_selector = KERNEL_LDT;
        tss.ss0 = KERNEL_DS;
        tss.esp0 = 0x800000;
        ltr(KERNEL_TSS);
    }

    /* Load the IDT in the IDTR register */
    lidt(idt_desc_ptr);

    /* Load all exceptions into the IDT */
    load_exceptions();

    /* Load system call entry in the IDT */
    load_system_call();

    /* Setup paging */
    init_paging();

    /* Init the PIC */
    i8259_init();

    reset_pids();

    /* Init the RTC */
    rtc_init();
    //rtc_open();
    /* Init the PS/2 controller */
    setup_ps2_controller();

    init_mouse();

    irq_t rtc;
    rtc.irq_num = RTC_IRQ;                  /* IRQ number for RTC   */
    rtc.handler = rtc_helper;               /* RTC handler          */
    init_interrupt(rtc);                    /* Start RTC interrupts */

    irq_t keyboard;
    keyboard.irq_num = KEYBOARD_IRQ;         /* IRQ number for keyboard */
    keyboard.handler = keyboard_helper; /* keyboard hanlder        */
    init_interrupt(keyboard);                /* Start keyboard interrupts */

    irq_t pit;
    pit.irq_num = 0;                  /* IRQ number for RTC   */
    pit.handler = pit_helper;               /* RTC handler          */
    init_interrupt(pit);                    /* Start RTC interrupts */

    irq_t mouse;
    mouse.irq_num = 12;
    mouse.handler = mouse_helper;
    init_interrupt(mouse);

    irq_t net;
    net.irq_num = 11;                  /* IRQ number for RTC   */
    net.handler = networking_helper;               /* RTC handler          */
    init_interrupt(net);                    /* Start RTC interrupts */

    irq_t sb16;
    sb16.irq_num = 5;
    sb16.handler = sound_helper;
    init_interrupt(sb16);

    // initialize all terminals and clear video memory
    clear();
    unsigned i;
    for (i = 0; i < NUM_TERMINALS; i++) {
      init_terminal(&terminals[i]);
    }

    // terminal_open();

    mount();

    // reset_fs();

    append_fs();

    init_signals();

    networking_init();

    pit_init();

    uint32_t loc = setup_graphics_mode();
    memset((void *)loc, 0xFF, 1280*1024*4);
    setup_BLT();

    graphic_terminal_open();

    load_gui_assets();

    num_active_terminals = 0;
    curterm = -1;
    process_pcb = NULL;

    // memcpy(loc + 4*1024*1024-1-16000 - 256*16, font_data, 256*16);
    // load terminal and bg into video memory
    // char * name = "terminal.txt";
    // dentry_ext_t den;
    // find_dentry_ext(name, &den, 7);

    // char buffer[1024];
    // uint32_t counter = 0;
    // int length = 32000;
    // while(length > 0){

    //     read_data_ext(den.inode, 1024*counter, buffer, 1024);
    //     memcpy(videomem + 4*1024*1024-1-32000+counter*1024, buffer, 1024);
    //     length -= 1024;
    //     counter++;

    // }

    // graphic_printf("LOADED TERMINAL \n");

    // name = "bg.txt";
    // if(-1 == find_dentry_ext(name, &den, 7)){
    //     graphic_printf("WTF...\n");
    // }

    // counter = 0;
    // length = read_length_ext(den.inode);
    // graphic_printf("BG LENGTH = %d\n", length);
    // while(length > 0){

    //     read_data_ext(den.inode, 1024*counter, buffer, 1024);
    //     memcpy(videomem + 1280*1024*2+counter*1024, buffer, 1024);
    //     length -= 1024;
    //     counter++;

    // }

    // graphic_printf("LOADED BG \n");

    // outb(0x0D, 0x3D4);
    // outb(0xFF, 0x3D5);
    // outb(0x0C, 0x3D4);
    // outb(0xFF, 0x3D5);

    // blt_operation_mmio(0, 0, 1280*2 - 1, 1024 - 1, 1280*2, 1280*2, 0, 1280*1024*2, 0, 0, BLT_DST_ROP, 0, 0);

    gui_init();

    // blt_operation_mmio(0, 0, 1280*2 - 1, 1024 - 1, 1280*2, 1280*2, 0, 4*1024*1024-1, 0, 0, BLT_DST_ROP, 0, 0);

    sti();

    /* Enable interrupts    */

    //list_directory_ext(0);
    // char * name = "file";
    // make_device_file_ext(name, )

    // execute_new_term((uint8_t*) "shell", 0);

    /* Initialize devices, memory, filesystem, enable device interrupts on the
     * PIC, any other initialization stuff... */

    /* Enable interrupts */
    /* Do not enable the following until after you have set up your
     * IDT correctly otherwise QEMU will triple fault and simple close
     * without showing you any output */
    /*printf("Enabling Interrupts\n");
    sti();*/
#ifdef RUN_TESTS
    /* Run tests */
    // launch_tests();
#endif
    /* Execute the first program ("shell") ... */

    /* Spin (nicely, so we don't chew up cycles) */
    asm volatile (".1: hlt; jmp .1;");
}
