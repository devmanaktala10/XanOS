#include "interrupt.h"
/*
 * init_interrupt
 *   DESCRIPTION: initialises interrupts from a specific irq number
 *   INPUTS: irq - an irq struct containing info of the irq handler and number
 *   OUTPUTS: void
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes the irq struct array. Enables irq on the master/slave PIC
 */
void init_interrupt(irq_t irq){

    /* add the irq struct to the array */
    irqs[irq.irq_num] = irq;

    /* set the IDT entry for irq       */
    idt_desc_t intr;
    intr.seg_selector = KERNEL_CS; /* Interrupts are handled in the Kernel */
    intr.reserved4 = 0;
    intr.reserved3 = 0;            /* Zero for Interrupts */
    intr.reserved2 = 1;
    intr.reserved1 = 1;
    intr.size      = 1;
    intr.reserved0 = 0;
    intr.dpl       = 0;             /* Privelege Level    */
    intr.present   = 1;
    SET_IDT_ENTRY(intr, irq.handler);
    /* ICW2_MASTER is the irq 0 IDT vector number */
    idt[ICW2_MASTER + irq.irq_num] = intr; /* Register in IDT */

    if(irq.irq_num >= RTC_IRQ){ /* RTC IRQ is the first slave IRQ so anything over that is in the slave */
        /* ICW3 specifies which irq slave is connected to */
        /* enable slave interrupts on master */
        enable_irq(ICW3_SLAVE);
        /* enable irq line */
        enable_irq(irq.irq_num);
    }
    else{
        /* enable irq line */
        enable_irq(irq.irq_num);
    }

}
