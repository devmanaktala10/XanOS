#include "signals.h"

int32_t sig_default_action(int32_t signum) {

  if (process_pcb->term != curterm) {
    schedule[curterm]->sigs.bitvector = (schedule[curterm]->sigs.bitvector | (1 << signum));
    return -1;
  }

  switch (signum) {
    case 0:
      graphic_printf("DIVISION BY ZERO \n");
      break;
    case 1:
      // graphic_printf("SEGFAULT \n");
      break;
    case 2:
      graphic_printf("SIGINT \n");
      break;
    case 3:
      graphic_printf("SIGALARM \n");
      break;
    case 4:
      graphic_printf("SIGUSER1 \n");
  }

  if (process_pcb->sigs.sig_ptr[signum] == &kill_process) {
    kill_process();
  } else if (process_pcb->sigs.sig_ptr[signum] == &ignore_sig) {
    ignore_sig();
    // return -1 for iret
    return -1;
  }

  return 0;
}

void kill_process() {
  // call halt
  graphic_printf("KILLING THE PROCESS \n");
  halt(1);
}

void ignore_sig() {
  graphic_printf("Ignoring the signal... \n");
}

void set_tss_args() {
  tss_args[0] = (uint32_t) tss.fs;
  tss_args[1] = (uint32_t) tss.es;
  tss_args[2] = (uint32_t) tss.ds;
}

void signal_handler_helper(int32_t signal_number, ptregs_t temp_ptregs) {

  // copy over ptregs structure into pcb
  // deep_copy_ptregs();

  uint32_t esp = temp_ptregs.esp;

  dentry_ext_t dentry;
  if(-1 == find_dentry_ext("siglink", &dentry, 8)) {
    // should kill program with error message
  }

  // get length of file and declare a buffer to copy data
  uint32_t length = read_length_ext(dentry.inode);
  uint8_t buf[length];

  // make space for data on stack
  esp -= length;

  // read data into buffer and copy to stack
  read_data_ext(dentry.inode, 0, (uint8_t*) buf, length);
  memcpy((uint8_t*) esp, (uint8_t*) buf, length);

  // get instruction pointer and modify it to point to start of siglink
  uint32_t eip = *(((uint32_t *)(buf) + 6));
  eip = eip - PROCESS_START + esp;

  // now push hardware context to user stack
  esp -= sizeof(ptregs_t);
  memcpy((uint8_t*) esp, &temp_ptregs, sizeof(ptregs_t));

  // save pointer to hardware_context into process_pcb to access in sigreturn
  process_pcb->sig_hw_esp = esp;

  // push signal signal_number
  esp -= 4;
  memcpy((uint8_t*) esp, &signal_number, 4);

  // push eip of siglink
  esp -= 4;
  memcpy((uint8_t*) esp, &eip, 4);

  // jump to handler provided by user
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
                "
                :
                :"a"(USER_DS), "b"(esp), "c"(USER_CS), "d"(process_pcb->sigs.sig_ptr[signal_number])
                :"memory", "cc"
  );

}

void execute_signals() {

  if (process_pcb == NULL) {
    return;
  }

  if (process_pcb->sigs.bitvector) {
    unsigned i;
    int32_t retval;
    for (i = 0; i < SIGNAL_NUM; i++) {
      if (process_pcb->sigs.bitvector & (1 << i)) {
        process_pcb->sigs.bitvector = (process_pcb->sigs.bitvector & (~(1 << i)));
        retval = sig_default_action(i);
        ptregs_t temp_ptregs;
        make_temp_ptregs(&temp_ptregs);
        temp_ptregs.sig_num = i;
        signal_handler_helper(i, temp_ptregs);
      }
    }
  }
}

void make_temp_ptregs(ptregs_t * temp_ptregs) {
  // temp_ptregs->ebx = tss.ebx;
  // temp_ptregs->ecx = tss.ecx;
  // temp_ptregs->edx = tss.edx;
  // temp_ptregs->esi = tss.esi;
  // temp_ptregs->edi = tss.edi;
  // temp_ptregs->ebp = tss.ebp;
  // temp_ptregs->eax = tss.eax;
  // temp_ptregs->ds = 0;
  // temp_ptregs->es = 0;
  // temp_ptregs->fs = 0;
  // temp_ptregs->sig_num = 0;
  // temp_ptregs->err_code = 0;
  // temp_ptregs->ret_add = tss.eip;
  // temp_ptregs->cs = 0;
  // temp_ptregs->eflags = tss.eflags;
  // temp_ptregs->esp = tss.esp;
  // temp_ptregs->ss = 0;

  int pid = process_pcb->pid;
  uint32_t sp = EIGHTMB- pid * (KERNEL_STACK_SIZE);
  
  // jump to handler provided by user
  asm volatile("                           \n\
                pushl %%ecx                 \n\
                movl -4(%%eax), %%ecx      \n\
                movl  %%ecx, 64(%%ebx)     \n\
                movl -8(%%eax), %%ecx      \n\
                movl  %%ecx, 60(%%ebx)     \n\
                movl -12(%%eax), %%ecx     \n\
                movl  %%ecx, 56(%%ebx)     \n\
                movl -16(%%eax), %%ecx     \n\
                movl  %%ecx, 52(%%ebx)     \n\   
                movl -20(%%eax), %%ecx     \n\
                movl  %%ecx, 48(%%ebx)     \n\
                movl -24(%%eax), %%ecx     \n\
                movl  %%ecx, 36(%%ebx)     \n\
                movl -28(%%eax), %%ecx     \n\
                movl  %%ecx, 32(%%ebx)     \n\
                movl -32(%%eax), %%ecx     \n\
                movl  %%ecx, 28(%%ebx)     \n\
                movl -36(%%eax), %%ecx     \n\
                movl  %%ecx, 24(%%ebx)     \n\   
                movl -40(%%eax), %%ecx     \n\
                movl  %%ecx, 20(%%ebx)     \n\
                movl -44(%%eax), %%ecx     \n\
                movl  %%ecx, 16(%%ebx)     \n\
                movl -48(%%eax), %%ecx     \n\
                movl  %%ecx, 12(%%ebx)     \n\
                movl -52(%%eax), %%ecx     \n\
                movl  %%ecx, 8(%%ebx)      \n\
                movl -56(%%eax), %%ecx     \n\
                movl  %%ecx, 4(%%ebx)      \n\
                movl -60(%%eax), %%ecx     \n\
                movl  %%ecx, (%%ebx)       \n\
                popl %%ecx                 \n\                                                                                                                                                                                                         
                "
                :
                :"a"(sp), "b"(temp_ptregs)
                :"memory", "cc"
  );

}

void sig_desc(int32_t signum){

  schedule[curterm]->sigs.bitvector = (schedule[curterm]->sigs.bitvector | (1 << signum));
  return;
  
}
