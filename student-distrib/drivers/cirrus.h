#ifndef _CIRRUS_H
#define _CIRRUS_H

#include "../types.h"
#include "../lib.h"
#include "../paging/page.h"
#include "./pci.h"

// VGA MODE REGISTERS
#define MM_PACKED 0x04
#define MM_DIRECT 0x06
#define SEG_GRAPH 0xA000
#define VGAREG_SEQU_ADDRESS 0x3c4
#define VGAREG_GRDC_ADDRESS 0x3ce
#define VGAREG_READ_MISC_OUTPUT 0x3cc
#define VGAREG_MDA_CRTC_ADDRESS 0x3b4
#define VGAREG_VGA_CRTC_ADDRESS 0x3d4
#define VGAREG_PEL_MASK 0x3c6
#define VGAREG_ACTL_ADDRESS 0x3c0
#define VGAREG_ACTL_WRITE_DATA  0x3c0
#define VGAREG_ACTL_READ_DATA   0x3c1
#define VGAREG_ACTL_RESET   0x3da
#define VGAREG_DAC_WRITE_ADDRESS 0x3c8
#define VGAREG_DAC_DATA 0x3c9

// BLT ENGINE ADRESS
#define BLT_ADDRESS 0x3CE
#define BLT_DATA    0x3CF

// BLT REGISTER OFFSETS
#define BACKGROUND_0 0x00
#define FOREGROUND_0 0x01
#define BACKGROUND_1 0x10
#define FOREGROUND_1 0x11
#define BACKGROUND_2 0x12
#define FOREGROUND_2 0x13
#define BACKGROUND_3 0x14
#define FOREGROUND_3 0x15
#define WIDTH_0 0x20
#define WIDTH_1 0x21
#define HEIGHT_0 0x22
#define HEIGHT_1 0x23
#define DESTINATION_PITCH_0 0x24
#define DESTINATION_PITCH_1 0x25
#define SOURCE_PITCH_0 0x26
#define SOURCE_PITCH_1 0x27
#define DESTINATION_START_0 0x28
#define DESTINATION_START_1 0x29
#define DESTINATION_START_2 0x2A
#define SOURCE_START_0 0x2C
#define SOURCE_START_1 0x2D
#define SOURCE_START_2 0x2E
#define DESTINATION_LEFT_SIDE_CLIPPING 0x2F
#define BLT_MODE 0x30
#define BLT_STATUS 0x31
#define BLT_ROP 0x32
#define BLT_MODE_EXTENSIONS 0x33
#define TRANSPARENT_0 0x34
#define TRANSPARENT_1 0x35

#define BACKGROUND_0_MMIO 0x00
#define BACKGROUND_1_MMIO 0x01
#define BACKGROUND_2_MMIO 0x02
#define BACKGROUND_3_MMIO 0x03
#define FOREGROUND_0_MMIO 0x04
#define FOREGROUND_1_MMIO 0x05
#define FOREGROUND_2_MMIO 0x06
#define FOREGROUND_3_MMIO 0x07
#define WIDTH_0_MMIO 0x08
#define WIDTH_1_MMIO 0x09
#define HEIGHT_0_MMIO 0x0A
#define HEIGHT_1_MMIO 0x0B
#define DEST_PITCH_0_MMIO 0x0C
#define DEST_PITCH_1_MMIO 0x0D
#define SRC_PITCH_0_MMIO 0x0E
#define SRC_PITCH_1_MMIO 0x0F
#define DEST_START_0_MMIO 0x10
#define DEST_START_1_MMIO 0x11
#define DEST_START_2_MMIO 0x12
#define SRC_START_0_MMIO 0x14
#define SRC_START_1_MMIO 0x15
#define SRC_START_2_MMIO 0x16
#define DEST_LSC_MMIO 0x17
#define BLT_MODE_MMIO 0x18
#define BLT_START_STATUS_MMIO 0x40
#define BLT_ROP_MMIO 0x1A
#define BLT_MODE_EXT_MMIO 0x1B
#define TRANS_0_MMIO 0x1C
#define TRANS_1_MMIO 0x1D

#define BLT_START_OPERATION 0x02
#define BLT_COLOR_EXPANSION 0x80
#define BLT_16BPP_EXPANSION 0x10
#define BLT_STATUS_BIT 0x01
#define BLT_DST_ROP 0x0D
#define BLT_ENABLE_TRANSPARENCY 0x08

struct vgamode_s {
    uint8_t memmodel;
    uint16_t width;
    uint16_t height;
    uint8_t depth;
    uint8_t cwidth;
    uint8_t cheight;
    uint16_t sstart;
};

typedef struct cirrus_mode {
    uint16_t mode, vesamode;
    struct vgamode_s info;

    uint16_t hidden_dac; /* 0x3c6 */
    uint16_t *seq; /* 0x3c4 */
    uint16_t *graph; /* 0x3ce */
    uint16_t *crtc; /* 0x3d4 */
} cirrus_mode_t;

void set_graphics_mode();
void blt_draw_image(uint32_t src, uint32_t dst, uint16_t width, uint16_t height, uint16_t src_pitch);
uint32_t setup_graphics_mode();
void blt_draw_image_color_expand(uint32_t background, uint32_t foreground, uint32_t src, uint32_t dst, uint16_t width, uint16_t height, uint16_t src_pitch);
void blt_write_raw_bytes(uint32_t src, uint32_t dst, uint16_t width, uint16_t height, uint16_t dst_pitch, uint16_t src_pitch);
void blt_operation_mmio(uint32_t background, uint32_t foreground, uint16_t width, uint16_t height, uint16_t dst_pitch, uint16_t src_pitch, uint32_t dst, uint32_t src, uint8_t dst_lsc, uint8_t mode, uint8_t rop, uint8_t mode_ext, uint16_t transparent);
void blt_operation_mmio_system(uint32_t background, uint32_t foreground, uint16_t width, uint16_t height, uint16_t dst_pitch, uint16_t src_pitch, uint32_t dst, uint32_t * src, uint8_t dst_lsc, uint8_t mode, uint8_t rop, uint8_t mode_ext, uint16_t transparent, uint32_t length);
void setup_BLT();

#endif
