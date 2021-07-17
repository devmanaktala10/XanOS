#ifndef _SOUND_H
#define _SOUND_H

#include "../types.h"
#include "../x86_desc.h"
#include "../lib.h"
#include "terminal.h"
#include "i8259.h"
#include  "../filesystem/filesystem.h"
#include "../filesystem/ext2.h"

#define SB16_IRQ 5
#define SPEAKER_ON 0xD1
#define SB_MEM (3*4*1024*1024)
#define NUM_WORDS 32*1024
#define NUM_BYTES 64*1024
#define DSP_RESET 0x226
#define DSP_READ_STATUS 0x22E
#define DSP_ACK_INT_8BIT 0x22E
#define DSP_ACK_INT_16BIT 0x22F
#define DSP_READ 0x22A
#define DSP_WRITE 0x22C
#define READ_TIMEOUT 10000

#define CH1_ADDR_PORT 0x02
#define CH1_COUNT_PORT 0x03
#define CH1_PAGE_PORT 0x83
#define CH5_ADDR_PORT 0xC4
#define CH5_COUNT_PORT 0xC6
#define CH5_PAGE_PORT 0X8B

#define MASK_PORT_8BIT 0x0A
#define TRANSFER_MODE_PORT_8BIT 0x0B
#define CLEAR_POINTER_8BIT 0x0C
#define MASK_PORT_16BIT 0xD4
#define TRANSFER_MODE_PORT_16BIT 0xD6
#define CLEAR_POINTER_16BIT 0xD8



#define READY_STATUS 0xAA

#define RESET_TIMEOUT 1000000

#define SET_OUTPUT_RATE 0x41
#define SET_INPUT_RATE 0x40

#define AUTO_TRANSFER_MODE 0xB6
#define STOP_PLAY 0xD5
#define GET_VERSION 0xE1

#define MASK_CH5 0x05
#define UNMASK_CH5 0x01

#define SC_PLAYBACK_MODE 0x49
#define AI_PLAYBACK_MODE 0x59
#define SINGLE_CYCLE_OUTPUT_8BIT 0xC0
#define SINGLE_CYCLE_OUTPUT_16BIT 0xB0
#define AI_OUTPUT_8BIT 0xC6
#define AI_OUTPUT_16BIT 0xB6
#define STEREO_SIGNED 0x30
#define MONO_UNSIGNED 0x00


#define TYPE_OFFSET 22
#define SAMPLE_OFFSET 24
#define BITS_OFFSET 34
#define PAUSE_8BIT 0xD0
#define RESUME_8BIT 0xD4
#define EOF_16BIT 0xD9
#define EOF_8BIT 0xDA
#define PAUSE_16BIT 0xD5
#define RESUME_16BIT 0xD6


typedef struct song {
    uint32_t inode_num;
    uint8_t type;
    uint32_t length;
    uint32_t length_read;
    uint32_t sample_rate;
    uint8_t bits;
} song_t;

int32_t play_song(uint8_t* file);
char read_DSP();
void program_DMA();
int32_t reset_DSP();
void program_DSP(uint32_t buf_length);
void sound_helper();
void sb16_interrupt_handler();
void load_song();
void double_buffer(uint32_t length);
void pause_song();
void resume_song();
int32_t sb16_ioctl_resume_song(unsigned long arg);
int32_t sb16_ioctl_pause_song(unsigned long arg);
int32_t sb16_ioctl_play_song(unsigned long arg);
int32_t sb16_ioctl(unsigned long cmd, unsigned long arg);
int32_t sb16_open();
int32_t sb16_close();
int32_t sb16_read(void * buf, int32_t nbytes);
int32_t sb16_write(const void * buf, int32_t nbytes);

#endif
