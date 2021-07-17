/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"

#define LSB_MASK 0x01
#define IRQ_PER_PIC 8

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */

/* void i8259_init(void)
 * Inputs: void
 * Return Value: void
 * Function: Sets up the master and slave 8259 PICs. */
void i8259_init(void) {

  master_mask = MASK_ALL;                  /* Mask to disable all interrupts */
  slave_mask  = MASK_ALL;

  unsigned long flags;
  cli_and_save(flags);                 /* Disable Interrupts and save flags */

  outb(master_mask, MASTER_8259_PORT + 1);        /* Mask all pic interrupts */
  outb(slave_mask, SLAVE_8259_PORT + 1);

  outb(ICW1, MASTER_8259_PORT);        /* Init Master PIC */
  outb(ICW2_MASTER, MASTER_8259_PORT + 1); /* Master IDT Vector Number */
  outb(ICW3_MASTER, MASTER_8259_PORT + 1); /* Bitmap of connected slaves */
  outb(ICW4, MASTER_8259_PORT + 1);        /* Normal EOI mode */

  outb(ICW1, SLAVE_8259_PORT);         /* Init Slave PIC */
  outb(ICW2_SLAVE, SLAVE_8259_PORT + 1);   /* Slave IDT Vector Number */
  outb(ICW3_SLAVE, SLAVE_8259_PORT + 1);   /* Slave on Pin:2 */
  outb(ICW4, SLAVE_8259_PORT + 1);         /* Normal EOI for Slave */

  outb(master_mask, MASTER_8259_PORT + 1);        /* Mask all pic interrupts */
  outb(slave_mask, SLAVE_8259_PORT + 1);

  restore_flags(flags);                /* Restore flags */

}

/* Enable (unmask) the specified IRQ */

/* void enable_irq(irq_num)
 * Inputs: irq_num - the irq number to be enabled
 * Return Value: void
 * Function: enables interrupts from the irq specified in the argument. */
void enable_irq(uint32_t irq_num) {

  unsigned long flags;
  uint8_t mask = LSB_MASK;

  cli_and_save(flags);

  if(irq_num >= IRQ_PER_PIC){ /* Mask Slave Interrupt */

    mask = mask << (irq_num - IRQ_PER_PIC);
    mask = ~(mask);
    slave_mask = slave_mask & mask;
    outb(slave_mask, SLAVE_8259_PORT + 1);

  }
  else{ //else its the master pic
    mask = mask << irq_num;
    mask = ~(mask);
    master_mask = master_mask & mask;
    outb(master_mask, MASTER_8259_PORT + 1);

  }
  restore_flags(flags);

}

/* Disable (mask) the specified IRQ */


/* void disnable_irq(irq_num)
 * Inputs: irq_num - the irq number to be disabled
 * Return Value: void
 * Function: disables interrupts from the irq specified in the argument. */
void disable_irq(uint32_t irq_num) {

  unsigned long flags;
  uint8_t mask = LSB_MASK;

  cli_and_save(flags);

  if(irq_num >= IRQ_PER_PIC){ /* Mask Slave Interrupt */

    mask = mask << (irq_num - IRQ_PER_PIC);
    slave_mask = slave_mask | mask;
    outb(slave_mask, SLAVE_8259_PORT + 1);

  }
  else{ //else its the master pic

    mask = mask << irq_num;
    master_mask = master_mask | mask;
    outb(master_mask, MASTER_8259_PORT + 1);

  }

  restore_flags(flags);

}

/* Send end-of-interrupt signal for the specified IRQ */

/* void send_eoi(irq_num)
 * Inputs: irq_num - the irq number for which eoi has to be sent
 * Return Value: void
 * Function: sends an EOI signal to the specified irq to enable more interrupts */
void send_eoi(uint32_t irq_num) {

  unsigned long flags;
  cli_and_save(flags);

  if(irq_num >= IRQ_PER_PIC){ //if irq number is greater than 7, its a slave pic

    outb(EOI | (irq_num - IRQ_PER_PIC), SLAVE_8259_PORT); // send EOI to master
    outb(EOI | ICW3_SLAVE, MASTER_8259_PORT); //send EOI to slave

  }
  else{ //else its the master pic

    outb(EOI + irq_num, MASTER_8259_PORT);
  }

  restore_flags(flags);

}
