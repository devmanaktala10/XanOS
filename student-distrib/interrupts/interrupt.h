#ifndef _INTERRUPT_H
#define _INTERRUPT_H

#include "../types.h"
#include "../x86_desc.h"
#include "../lib.h"
#include "../drivers/i8259.h"
#include "../drivers/rtc.h"

#define NUM_IRQS 16

/* Structure to store interrupts */
typedef struct irq{
    uint8_t irq_num;            /* IRQ number       */
    void * handler;         /* Handler Pointer  */
} irq_t;

/* Interrupt Functions */
void init_interrupt(irq_t irq);

/* structure to store interrupt info */
irq_t irqs[NUM_IRQS];

#endif
