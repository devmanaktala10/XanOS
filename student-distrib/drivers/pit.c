#include "pit.h"

void pit_init(){
  outb(SET_MODE, MODE_REG);
  outb(0xFF,CH0_DATA_PORT);
  outb(0xFF,CH0_DATA_PORT);
}

void pit_handler(){
  unsigned long flags;
  cli_and_save(flags);
  disable_irq(0);
  send_eoi(0);
  //printft("int occurred");

  //RESCHEDULE HERE

  enable_irq(0);

  render_gui();
  
  sched_process();

  execute_gui_handlers();

  execute_signals();

  // blt_operation_mmio(0xFFFFFFFF, 0xFEFEFEFE, 32 - 1, 32 - 1, 1280, 32, pos_x + pos_y * 1280, 3*1024*1024, 0, BLT_COLOR_EXPANSION | 0x01 | 0x08, BLT_DST_ROP, 0, 0);

  restore_flags(flags);
}
