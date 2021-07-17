#ifndef _EXT2_
#define _EXT2_

#include "filesystem.h"
#include "../lib.h"
#include "../drivers/ata.h"

#define EXT_MAX_FILENAME 32
#define DBLOCKS_PER_INODE 254
#define INDIRECT_BLOCK    254
#define DOUBLE_INDIRECT_BLOCK 255

#define READ_LOCK 0x01
#define WRITE_LOCK 0x02
#define BLOCK_SIZE 1024
#define SUPER_BLOCK 0
#define BASE_DIR_OFFSET 0
#define MAX_ALLOCATED_BUFFERS 80

#define SLAB_START 4*1024*1024*1023

uint32_t BLOCK_SIZE_EXT;
uint32_t NUM_BLOCKS_EXT;
uint32_t NUM_INODES_EXT;
uint32_t NUM_DATA_BLOCKS_EXT;
uint32_t INODE_BITMAP_LOCATION;
uint32_t DATA_BITMAP_LOCATION;
uint32_t INODE_0_LOCATION;
uint32_t DATA_0_LOCATION;
uint32_t NUM_FREE_DATA_BLOCKS;

uint32_t curdir;
uint32_t lastfreeblock;
uint32_t lastfreeinode;

char alloc_bitmap[10];
uint32_t last_alloc;

char inodemap[1024];
int lastdatablock;
char datamap[1024];

typedef struct dentry_ext {

    char filename[EXT_MAX_FILENAME];
    uint32_t inode;
    uint32_t filetype;
    uint32_t present;

} dentry_ext_t;

void mount();
void read_block(uint32_t idx, char * buffer);
void write_block(uint32_t idx, char * buffer);
int get_inode_bitmap(uint32_t idx);
void set_inode_bitmap(uint32_t idx, uint32_t val);
int make_empty_file_ext(char * buffer, uint32_t dir_inode);
int read_current_directory(uint32_t idx, char * name, uint32_t dir_inode);
void list_directory_ext(uint32_t dir_inode);
int make_directory(char * name, uint32_t dir_inode);
int set_current_directory(char * buffer);
void reset_fs();
int find_dentry_ext(char * name, dentry_ext_t * dentry, uint32_t dir_inode);
int find_empty_datablock();
void set_data_bitmap(uint32_t idx, uint32_t val);
int get_data_bitmap(uint32_t idx);
void reset_buffer(char * buffer);
int parse_path(char * path, uint32_t dir_inode, char * name);
uint32_t get_free_blocks();
void set_free_blocks(uint32_t new_free_blocks);
void cleanup_file_ext(uint32_t file_inode);
int delete_file_ext(uint32_t file_inode, uint32_t dir_inode);
int32_t write_data_ext(uint32_t inode, uint8_t * buf, uint32_t offset, uint32_t length);
uint32_t write_partial_block(uint32_t inode, uint8_t * buf, uint32_t offset, uint32_t length);
int32_t read_data_ext(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
uint32_t read_partial_block(uint32_t inode, uint32_t offset, uint8_t * buf, uint32_t length);
int read_dentry_by_index_ext(uint32_t index, dentry_ext_t * d, uint32_t dir_num);
uint32_t read_length_ext(uint32_t inode);
int make_device_file_ext(char * name, uint32_t dir_inode);
uint32_t read_complete_block(uint32_t inode, uint32_t offset, uint8_t * buf, uint32_t length);
uint32_t read_partial_block_end(uint32_t inode, uint32_t offset, uint8_t * buf, uint32_t length);
int delete_dir(uint32_t dir_num);
char * get_allocated_block();
void free_allocated_block(char * block);
void append_fs();

void initext();

#endif
