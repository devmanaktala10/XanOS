#include "rtc.h"

char frequency_to_rate[NUM_FREQ]={15,14,13,12,11,10,9,8,7,6};
static volatile int interrupt_occured=0;

/* void rtc_init(void)
 * Inputs: void
 * Return Value: void
 * Function: sets up recurring rtc interrupts at irq8 */
void rtc_init(void){
  unsigned long flags;
  cli_and_save(flags);

  outb(RTC_REGB | RTC_NMI, REGISTER_PORT); //select register B, disables NMI
  char cur = inb(RW_PORT); //get the current value
  outb(RTC_REGB | RTC_NMI, REGISTER_PORT); //select regsiter B
  outb(cur|BIT6_MASK, RW_PORT); // turn on bit 6 of register B
  outb(inb(REGISTER_PORT) & BIT8_MASK, REGISTER_PORT); //enable NMI again
  restore_flags(flags);
}

/* rtc_interrupt_handler
 * DESCRIPTION: Interrupt handler function for RTC interrupts.
 * INPUTS: None
 * OUTPUTS:None
 * SIDE EFFECTS: None */
void rtc_interrupt_handler(){

  unsigned long flags;
  cli_and_save(flags);
  //mask interrupts
  disable_irq(RTC_IRQ);
  /* send EOI signal to PIC */
  send_eoi(RTC_IRQ);

  // printft("1");

  /* volatile variable for read */
  interrupt_occured=1;
  /* write to register C to get next interrupt */
  outb(RTC_REGC,REGISTER_PORT);

  /* read from data port */
  inb(RW_PORT);
  //unmask interrupt
  enable_irq(RTC_IRQ);

  restore_flags(flags);

}

/* int32_t rtc_open
 * DESCRIPTION: Sets the RTC interrupt frequency to 2Hz.
 * RETURN VALUE: always 0
 * SIDE EFFECTS: Changes RTC interrupt frequency
 * INPUT: Ignored
 * OUTPUT: None */
int32_t rtc_open(void* buf, int32_t length) {
  unsigned long flags;
  cli_and_save(flags);

  /* write_rtc 2hz as the frequency */
  write_rtc(OPEN_FREQ);

  restore_flags(flags);

  /* return 0 when complete */
  return 0;
}

int32_t rtc_write(const void* buf, int32_t nbytes) {

  uint32_t freq = *((uint32_t*) buf);
  return write_rtc(freq);
}

/* int32_t write
 * DESCRIPTION: Sets the frequency to the input frequenct if the frequency is a power of 2
 *              and between 2 and 1024Hz. The rate is determined by: Frequency = 32768 >> (rate - 1)
 *              Write to the rtc ports to set the rate.
 * RETURN VALUE: uint32_t: The number of bytes written
 * SIDE EFFECTS: Changes RTC interrupt frequency
 * INPUT: uint32_t frequency: frequency to set the rtc interrupts to
 * OUTPUT: None */
int32_t write_rtc(uint32_t frequency){

  /*TODO: Check input for power of 2, return -1 on failure */
  uint16_t index;
  uint16_t temp=1;
  char rate;
  /* Maximum value of rate is 15 */
  for(index=0; temp>0; temp=temp<<1, index++){

    /* if rate is a power of 2, break */
    if(temp==frequency)
      break;

    else if(index>temp) return -1; //if index

  }
  if(index>=1 && index<=10) //if index is in an allowable range
  rate=frequency_to_rate[index-1]; //get rate from power of two of the frequency.

  else return -1; //else frequency is out of bounds

  unsigned long flags;
  cli_and_save(flags);

  /* Read register A to get current value of register*/
  outb(RTC_REGA, REGISTER_PORT);
  char cur = inb(RW_PORT);

  /* Write to Register A to send rate in next write */
  outb(RTC_REGA, REGISTER_PORT);
  char newrate=(MASK_4&cur)|rate;

  /* Set the new rate */
  outb(newrate, RW_PORT);

  restore_flags(flags);

  /* Total bytes written = 3 */
  return 3;

}

/* int32_t rtc_read
 * DESCRIPTION: Waits the next interrupt to be occured. The interrupt handler sets
 *              the volatile interrupt_occured variable to 1 when an rtc interrupt
 *              occurs.
 * RETURN VALUE: int32_t always 0
 * SIDE EFFECTS: None
 * INPUT: None
 * OUTPUT: None */
int32_t rtc_read(){

  /* Clear the interrupt occured variable */
  unsigned long flags;
  cli_and_save(flags);
  interrupt_occured=0;
  restore_flags(flags);

  /* wait for interrupt handler to set this flag */
  while(!interrupt_occured){
    // printft("Waiting for rtc interrupt to occur\n");
  }
  cli_and_save(flags);
  // printft("interrupt occured\n");

  /* Clear the interrupt occured variable */
  interrupt_occured=0;

  restore_flags(flags);

  return 0;
}

int32_t rtc_close(){
  return -1;
}
/* TODO: WRITE UNIT TESTS */

int32_t rtc_ioctl(unsigned long cmd, unsigned long arg){
  return -1;
}
