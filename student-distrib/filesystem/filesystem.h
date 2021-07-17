#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H

#define MAX_DENTRYS 63
#define MAX_DATA_BLOCKS_PER_FILE 1023
#define MAX_FILENAME_LENGTH 32

#include "../types.h"
#include "../multiboot.h"
#include "../drivers/keyboard.h"
#include "../lib.h"
#include "../drivers/rtc.h"
#include "../processes/process.h"
#include "../drivers/ata.h"
#include "ext2.h"

#define SIZE_OF_BLOCK   (4 * 1024)
#define DENTRY_BLOCK    16
#define INODE_OFF       1
#define DATA_OFF        2
#define FILES_TO_TRACK  8

typedef struct dentry
{
    char filename[MAX_FILENAME_LENGTH];
    uint32_t filetype;
    uint32_t inode_num;

} dentry_t;

uint32_t NUM_DIR_ENTRIES;
uint32_t NUM_INODES;
uint32_t NUM_DATA_BLOCKS;
uint32_t * fs_base;

void init_fs(uint32_t * fs_start);
int32_t read_dentry_by_name(const uint8_t * fname, dentry_t * dentry);
int32_t get_dentry_index_by_name(const uint8_t * fname, dentry_t * dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
uint32_t read_length(uint32_t inode);

int32_t file_open();
int32_t file_read(void* buf, int32_t length);
int32_t file_close();
int32_t file_write(const void* buf, int32_t length);
int32_t file_ioctl(unsigned long cmd, unsigned long arg);

int32_t dir_open();
int32_t dir_read(void* buf, int32_t length);
int32_t dir_close();
int32_t dir_write(const void* buf, int32_t length);
int32_t dir_ioctl(unsigned long cmd, unsigned long arg);

#endif
