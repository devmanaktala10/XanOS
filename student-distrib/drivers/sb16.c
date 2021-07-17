#include "sb16.h"

volatile int interrupt_counter=0;
volatile song_t cur_song;
uint8_t offset_port;
uint8_t page_port;
uint8_t count_port;
uint16_t ack_port;
uint8_t mask_port;
uint8_t ff_port;
uint8_t mode_port;
uint8_t command;
uint8_t pause;
uint8_t resume;
uint8_t eof_resp;
volatile int eof = 0;


int32_t play_song(uint8_t* file){

  unsigned long flags;
  cli_and_save(flags);
  dentry_t song;
  eof = 0;

  uint32_t dir;
  if(process_pcb == NULL){
    dir= 0;
  }
  else{
    dir = process_pcb->curdir;
  }

  char name[32];
  reset_buffer(name);
  int ret = parse_path(file, dir, name);

  if(ret == -1){ //invalid path
    restore_flags(flags);
    return -1;
  }
    
  dentry_ext_t dentry;
  if(-1 == find_dentry_ext(name, &dentry, ret)){ //inavlid filename
      restore_flags(flags);
      return -1;
  }


  if(reset_DSP() == -1){

    //graphic_printf("Sound Blaster 16 not present\n");
    restore_flags(flags);
    return -1;

  }

  cur_song.inode_num = dentry.inode;

  //read header
  char buf[44];
  int read = read_data_ext(cur_song.inode_num, 0, buf, 44);

  if(read != 44) {

    // unexpected idk
    restore_flags(flags);
    return -1;

  }

  memcpy(&cur_song.type, buf + TYPE_OFFSET, 2);
  memcpy(&cur_song.sample_rate, buf + SAMPLE_OFFSET, 4);
  memcpy(&cur_song.bits, buf + BITS_OFFSET, 2);

  // if(read_dentry_by_name(file,&song) == -1){
  //   printf("file not found\n");
  //   restore_flags(flags);
  //   return -1;
  // }

  // read_data(cur_song.inode_num, TYPE_OFFSET, &cur_song.type, 2);
  // read_data(cur_song.inode_num, SAMPLE_OFFSET, &cur_song.sample_rate, 4);
  // read_data(cur_song.inode_num, BITS_OFFSET, &cur_song.bits, 2);

  cur_song.length = read_length_ext(cur_song.inode_num) - 44;
  cur_song.length_read = 0;

  switch(cur_song.bits){
    case 8:
      offset_port = CH1_ADDR_PORT;
      page_port = CH1_PAGE_PORT;
      count_port = CH1_COUNT_PORT;
      ack_port = DSP_ACK_INT_8BIT;
      mask_port = MASK_PORT_8BIT;
      ff_port = CLEAR_POINTER_8BIT;
      command = AI_OUTPUT_8BIT;
      mode_port = TRANSFER_MODE_PORT_8BIT;
      pause = PAUSE_8BIT;
      resume = RESUME_8BIT;
      eof_resp = EOF_8BIT;
      break;
    case 16:
      offset_port = CH5_ADDR_PORT;
      page_port = CH5_PAGE_PORT;
      count_port = CH5_COUNT_PORT;
      ack_port = DSP_ACK_INT_16BIT;
      mask_port = MASK_PORT_16BIT;
      ff_port = CLEAR_POINTER_16BIT;
      command = AI_OUTPUT_16BIT;
      mode_port = TRANSFER_MODE_PORT_16BIT;
      pause = PAUSE_16BIT;
      resume = RESUME_16BIT;
      eof_resp = EOF_16BIT;
      break;
    default:
      offset_port = CH1_ADDR_PORT;
      page_port = CH1_PAGE_PORT;
      count_port = CH1_COUNT_PORT;
      ack_port = DSP_ACK_INT_8BIT;
      mask_port = MASK_PORT_8BIT;
      ff_port = CLEAR_POINTER_8BIT;
      command = AI_OUTPUT_8BIT;
      mode_port = TRANSFER_MODE_PORT_8BIT;
      pause = PAUSE_8BIT;
      resume = RESUME_8BIT;
      eof_resp = EOF_8BIT;
      break;
  }

  interrupt_counter = 0;
  restore_flags(flags);

  load_song();

  cli_and_save(flags);
  program_DMA();
  set_rate();
  program_DSP(NUM_BYTES);
  enable_irq(SB16_IRQ);

  restore_flags(flags);

  return 0;
}

int32_t reset_DSP(){
  //reset DSP
  outb(1, DSP_RESET);
  int i;
  for(i=0;i < 10000;i++);
  outb(0, DSP_RESET);

//wait for read from the register till RESET ACK is recieved
  uint8_t ready = 0xFF;
  i = 0;

  while(ready != READY_STATUS && i != RESET_TIMEOUT){
    ready = read_DSP();
    i++;
  }

  if(i == RESET_TIMEOUT) {
    return -1;
  }
}

void program_DSP(uint32_t buf_length){
//Write command
  while( (inb(DSP_WRITE) >> 7) & 0x01);
  outb(command, DSP_WRITE);

  //Write mode
  uint8_t mode = 0x00|((cur_song.type & 0x02) << 4);
  if(cur_song.bits == 16){
    mode |= 0x10;
  }
  while( (inb(DSP_WRITE) >> 7) & 0x01);
  outb(mode, DSP_WRITE);

  //write length
  uint8_t high = (((buf_length/(cur_song.bits >> 3)/2) - 1) >> 8);
  uint8_t low = (buf_length/(cur_song.bits >> 3)/2) - 1;

while( (inb(DSP_WRITE) >> 7) & 0x01);
outb(low, DSP_WRITE);

while( (inb(DSP_WRITE) >> 7) & 0x01);
outb(high, DSP_WRITE);

while( (inb(DSP_WRITE) >> 7) & 0x01);
outb(SPEAKER_ON, DSP_WRITE);

}

char read_DSP(){
  int i = 0;

  //Wait for the read status to be ready
  while((inb(DSP_READ_STATUS) & 0x80 == 0) && (i != READ_TIMEOUT))
    i++;

  if(i == READ_TIMEOUT) return 0xFF;

  else
  return inb(DSP_READ);

}

void program_DMA(){
  //reference http://qzx.com/pc-gpe/sbdsp.txt

  //mask channel 5
  outb(MASK_CH5, mask_port);

  //clear flip flop
  outb(0x00, ff_port);

  //SELECT playback mode on channel 5
  outb(AI_PLAYBACK_MODE, mode_port);

  //write offset address
  uint16_t bufoff = ((SB_MEM)/2) % 65536;
  uint8_t high = (bufoff >> 8);
  uint8_t low = (bufoff & 0x00ff);

  outb(0, offset_port);
  outb(0, offset_port);

  //outb(0x00, CLEAR_POINTER_8BIT);

  //Write length of buffer
  high = ((NUM_BYTES/(cur_song.bits >> 3) - 1) >> 8);
  low = NUM_BYTES/(cur_song.bits >> 3) - 1;
  outb(low, count_port);
  outb(high, count_port);

  //write page port
  uint8_t page = (SB_MEM / 65536);

  outb(page, page_port);

  //enable channel 5
  outb(UNMASK_CH5, mask_port);

}

void set_rate(){
  while(((inb(DSP_WRITE) >> 7) & 0x01) != 0);
  outb(SET_OUTPUT_RATE, DSP_WRITE);

  while(((inb(DSP_WRITE) >> 7) & 0x01) != 0);
  outb(((cur_song.sample_rate & 0xFF00)>>8), DSP_WRITE);

  while(((inb(DSP_WRITE) >> 7) & 0x01) != 0);
  outb((cur_song.sample_rate & 0x00FF), DSP_WRITE);
}

void sb16_interrupt_handler(){
  unsigned long flags;
  cli_and_save(flags);
  disable_irq(SB16_IRQ);
  interrupt_counter++;
  // graphic_printf("INTERRUPT OCCURRED %d\n", interrupt_counter);
  if(eof){
    while( (inb(DSP_WRITE) >> 7) & 0x01);
    outb(eof_resp, DSP_WRITE);
    inb(ack_port);
    send_eoi(SB16_IRQ);
    enable_irq(SB16_IRQ);
    restore_flags(flags);
    return;
  }
  uint32_t next_block;
  cur_song.length_read += 32*1024;
  if(cur_song.length - cur_song.length_read < 32*1024){
    next_block = cur_song.length - cur_song.length_read;
    program_DSP(next_block);
    eof = 1;
  }
  else next_block = 32*1024;

  inb(ack_port);
  send_eoi(SB16_IRQ);

  double_buffer(next_block);
  enable_irq(SB16_IRQ);
  restore_flags(flags);
  return 0;
}

void load_song(){
  uint32_t length = cur_song.length;
  char * buf = get_allocated_block();
  uint32_t counter = 0;
  while(length > 0){

    uint32_t read = read_data_ext(cur_song.inode_num, 44+1024*counter, buf, 1024);
    memcpy((uint32_t*)(SB_MEM + counter*1024), buf, read);
    counter++;
    length -= read;

  }

  free_allocated_block(buf);
}

void double_buffer(uint32_t length){
  uint32_t counter =0;
  char buf[1024];
  if(interrupt_counter!=0){
  while(length > 0){
    if (length < 1024){
      memcpy((uint32_t*)(SB_MEM + 32*1024 * (1-(interrupt_counter%2)) + counter*1024),(uint32_t*) (SB_MEM + 64*1024 + 32*1024*(interrupt_counter-1) + counter*1024), length);
      break;
    }
      memcpy((uint32_t*)(SB_MEM + 32*1024 * (1-(interrupt_counter%2)) + counter*1024),(uint32_t*)(SB_MEM + 64*1024 + 32*1024*(interrupt_counter-1) + counter*1024), 1024);
    length -= 1024;
    counter++;
  }
}
}

void pause_song(){
  unsigned long flags;
  cli_and_save(flags);
  disable_irq(SB16_IRQ);
  while( (inb(DSP_WRITE) >> 7) & 0x01);
  outb(pause, DSP_WRITE);
  enable_irq(SB16_IRQ);
  restore_flags(flags);
}

void resume_song(){
  unsigned long flags;
  cli_and_save(flags);
  disable_irq(SB16_IRQ);
  while( (inb(DSP_WRITE) >> 7) & 0x01);
  outb(resume, DSP_WRITE);
  enable_irq(SB16_IRQ);
  restore_flags(flags);
}

int32_t sb16_ioctl(unsigned long cmd, unsigned long arg){

    switch(cmd){

        case 0:
            return sb16_ioctl_play_song(arg);
        case 1:
            return sb16_ioctl_pause_song(arg);
        case 2:
            return sb16_ioctl_resume_song(arg);
        default:
            return -1;

    }

}

int32_t sb16_ioctl_play_song(unsigned long arg){

    char * path = (char *)arg;
    return play_song(path);

}

int32_t sb16_ioctl_pause_song(unsigned long arg){

    pause_song();
    return 0;

}

int32_t sb16_ioctl_resume_song(unsigned long arg){

    resume_song();
    return 0;

}

int32_t sb16_open(){

  return 0;

}
int32_t sb16_close(){

  return -1;

}
int32_t sb16_read(void * buf, int32_t nbytes){

  return -1;

}

int32_t sb16_write(const void * buf, int32_t nbytes){

  return -1;

}