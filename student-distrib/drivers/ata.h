#ifndef _ATA_H
#define _ATA_H

#include "../lib.h"

/* IDE base port */
#define IDE_BASE_PORT 0x1F0
#define IDE_CONTROL_BASE 0x3F7

/* ATA register offsets */
#define DATA_REGISTER 0
#define ERROR_REGISTER 1
#define SELECT_COUNT_REGISTER 2
#define SELECT_NUMBER_REGISTER 3
#define SELECT_LOW_REGISTER 4
#define SELECT_HIGH_REGISTER 5
#define SELECT_DRIVE_REGISTER 6
#define SELECT_STATUS_COMMAND_REGISTER 7
#define PRIMARY_IRQ_NUM 14

/* Hard disk B */
#define HDB 0xB0

/* initialize the device */
void init_ata();
int read_sector(uint32_t lba, char * buffer);
int write_sector(uint32_t lba, char * buffer);

#endif
