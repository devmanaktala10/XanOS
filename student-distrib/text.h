#ifndef _TEXT_H
#define _TEXT_H

#include "types.h"

typedef struct ptregs {
  uint32_t ebx; // 0
  uint32_t ecx; // 4
  uint32_t edx; // 8
  uint32_t esi; // 12
  uint32_t edi; // 16
  uint32_t ebp; // 20
  uint32_t eax; // 24

  uint32_t ds; // 28
  uint32_t es; // 32
  uint32_t fs; // 36

  uint32_t sig_num; // 40
  uint32_t err_code; // 44
  uint32_t ret_add; // 48
  uint32_t cs; // 52
  uint32_t eflags; // 56
  uint32_t esp; // 60
  uint32_t ss; // 64
} ptregs_t;

typedef struct mouse_args{

    uint8_t byte0;
    uint8_t byte1;
    uint8_t byte2;

} mouse_args_t;

extern unsigned char font_data[256][16];
void char_to_buffer(char s, unsigned char * buffer);

#endif
