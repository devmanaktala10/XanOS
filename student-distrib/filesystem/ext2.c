#include "ext2.h"

/* parse the superblock and store the info */

/* super block format */
/* long 1: block size */
/* long 2: # blocks */
/* long 3: # inodes */
/* long 4: # data blocks */
/* mount
 * DESCRIPTION: Mount the ext filesystem and read the first block containing the bookkeeping info.
 * INPUTS: None
 * OUTPUT: None
 * SIDE EFFECTS: Initialize filesystem values
 */
void mount(){

    char buffer[BLOCK_SIZE];
    /* read block 0 */
    read_block(SUPER_BLOCK, buffer);
    /* get values from block 0 */
    BLOCK_SIZE_EXT = *(((uint32_t *) buffer));
    NUM_BLOCKS_EXT = *(((uint32_t *) buffer) + 1);
    NUM_INODES_EXT = *(((uint32_t *) buffer) + 2);
    NUM_DATA_BLOCKS_EXT = *(((uint32_t *) buffer) + 3);
    INODE_BITMAP_LOCATION = *(((uint32_t *) buffer) + 4);
    DATA_BITMAP_LOCATION = *(((uint32_t *) buffer) + 5);
    INODE_0_LOCATION = *(((uint32_t *) buffer) + 6);
    DATA_0_LOCATION = *(((uint32_t *) buffer) + 7);
    NUM_FREE_DATA_BLOCKS = *(((uint32_t *) buffer) + 8);

    /* curdir is the first inode '/' */
    curdir = 0;
    lastfreeblock = 0;
    lastfreeinode = 0;
    lastdatablock = -1;
    last_alloc = 0;

    page_dir[200].p = 1;
    page_dir[200].ps = 1;
    page_dir[200].base_address = FOURKB_ADDRESS((512-1)/4*1024*1024);
    set_cr((void *)page_dir);

    int i = 0;
    for(i = 0; i < 10; i++) alloc_bitmap[i] = 0x00;

}

char * get_allocated_block(){

    unsigned long flags;
    cli_and_save(flags);

    int i;
    uint32_t start = last_alloc;
    for(i = 0; i < MAX_ALLOCATED_BUFFERS; i++){

        uint32_t index = start/8;
        uint32_t shift = start%8;

        if((alloc_bitmap[index] & (0x01 << shift)) == 0){

            last_alloc++;
            restore_flags(flags);
            return ((4*1024*1024*200 + 1024*start));

        }

        start++;
        start = start%80;

    }

    restore_flags(flags);
    return NULL;

}

void free_allocated_block(char * block){

    unsigned long flags;
    cli_and_save(flags);

    uint32_t start = (((uint32_t)block) - 4*1024*1024*200)/1024;
    uint32_t index = start/8;
    uint32_t shift = start%8;

    alloc_bitmap[index] = alloc_bitmap[index] & (~(0x01 << shift));
    last_alloc = start;

    restore_flags(flags);

}

void read_block(uint32_t idx, char * buffer){

    uint32_t lba = idx * 2;
    if(lba >= 1024*1024*1024/512) return;
    while(0 != read_sector(lba, buffer));
    lba += 1;
    while(0 != read_sector(lba, (buffer + 512)));

}

void write_block(uint32_t idx, char * buffer){

    uint32_t lba = idx * 2;
    if(lba >= 1024*1024*1024/512) return;
    while(1){
        while(0 != write_sector(lba, buffer));
        char buf[512];
        while(0 != read_sector(lba, buf));
        if(strncmp(buf, buffer,512) == 0) break;
    }
    lba += 1;
    while(1){
        while(0 != write_sector(lba, (buffer + 512)));
        char buf[512];
        while(0 != read_sector(lba, buf));
        if(strncmp(buf, (buffer + 512),512) == 0) break;
    }

}

int get_data_bitmap(uint32_t idx){

    unsigned long flags;
    cli_and_save(flags);

    uint32_t buf_idx = idx/(1024*8);
    uint32_t block_idx = (idx%(8*1024))/8;
    uint32_t char_idx = (idx%(8*1024))%8;

    if(lastdatablock != -1 && lastdatablock == buf_idx){

        if(datamap[block_idx] & (0x01 << char_idx)){
            return 0;
        }
        else{
            return -1;
        }

    }
    else{

        read_block(DATA_BITMAP_LOCATION + buf_idx, datamap);
        lastdatablock = buf_idx;

        if(datamap[block_idx] & (0x01 << char_idx)){
            return 0;
        }
        else{
            return -1;
        }

    }

    restore_flags(flags);
}

void set_data_bitmap(uint32_t idx, uint32_t val){

    uint32_t buf_idx = idx/(1024*8);
    uint32_t block_idx = (idx%(8*1024))/8;
    uint32_t char_idx = (idx%(8*1024))%8;

    unsigned long flags;
    cli_and_save(flags);

    if(!(lastdatablock != -1 && lastdatablock == buf_idx)){

        read_block(DATA_BITMAP_LOCATION + buf_idx, datamap);
        lastdatablock = buf_idx;

    }

    if(val == 1){
        datamap[block_idx] = datamap[block_idx] | (0x01 << char_idx);
    }
    else{
        datamap[block_idx] = datamap[block_idx] & (~(0x01 << char_idx));
    }
    write_block(DATA_BITMAP_LOCATION + buf_idx, datamap);
    restore_flags(flags);

}

int find_empty_datablock(){

    unsigned long flags;
    cli_and_save(flags);
    uint32_t i = 0;
    uint32_t start = lastfreeblock;
    while(i < 1016*1024){

        if(-1 == get_data_bitmap(start)){

            set_data_bitmap(start, 1);
            lastfreeblock = start;
            restore_flags(flags);
            return start;

        }
        start++;
        start = start%(1016*1024);
        i++;

    }
    restore_flags(flags);
    return -1;

}

int get_inode_bitmap(uint32_t idx){

    unsigned long flags;
    cli_and_save(flags);

    read_block(INODE_BITMAP_LOCATION, inodemap);
    uint32_t block_idx = idx/8;
    uint32_t char_idx = idx%8;

    if(inodemap[block_idx] & (0x01 << char_idx)){
        restore_flags(flags);
        return 0;
    }
    else{
        restore_flags(flags);
        return -1;
    }

}

void set_inode_bitmap(uint32_t idx, uint32_t val){

    unsigned long flags;
    cli_and_save(flags);

    read_block(INODE_BITMAP_LOCATION, inodemap);
    uint32_t block_idx = idx/8;
    uint32_t char_idx = idx%8;
    if(val == 1){
        inodemap[block_idx] = inodemap[block_idx] | (0x01 << char_idx);
    }
    else{
        inodemap[block_idx] = inodemap[block_idx] & (~(0x01 << char_idx));
    }
    write_block(INODE_BITMAP_LOCATION, inodemap);
    restore_flags(flags);

}

int find_empty_inode(){

    unsigned long flags;
    cli_and_save(flags);

    uint32_t i = 0;
    uint32_t start = lastfreeinode;
    while(i < 8*1024){

        if(-1 == get_inode_bitmap(start)){

            lastfreeinode = start;
            set_inode_bitmap(start, 1);
            restore_flags(flags);
            return start;

        }
        start++;
        start = start%(8*1024);
        i++;

    }

    restore_flags(flags);
    return -1;

}

/* each process should have its own curdir
 *
 * DIRECTORY:
 *
 * open: set current directory
 * close: close directory
 * read: filenames in directory
 * write: -1
 *
 * FILE:
 *
 * open: open file or create one
 * close: close file
 * read: read bytes from file
 * write: write bytes to file
 *
 * NEW SYSTEM CALLS
 *
 * mkdir: create directory at absolute path
 * rmdir: remove directory at absolute path
 * rm: remove file at absolute path
 *
 * LS: list current directory of PROCESS
 *     open "."
 *     while(read ".")
 * cd: set PROCESS dir to relative/absolute path
 *     open "path"
 *     close "path"???
 * mkdir: make new directory at path
 *     open path
 *     write dir_name
 *
 * rmdir: remove directory at path
 *     open path
 *     close dir_name
 * touch: make file at path
 *     open path
 *
 */

/* make an empty file with name in char array at directory specified by
 * directory inode */
int make_device_file_ext(char * name, uint32_t dir_inode){

    // no free inodes
    int inode = find_empty_inode();
    if(inode == -1) return -1;

    // filename exists
    dentry_ext_t gar;
    if(0 == find_dentry_ext(name, &gar, dir_inode)) {
        set_inode_bitmap (inode, 0);
        return -1;
    }

    dentry_ext_t d;
    int i;

    d.filetype = 2;
    d.present = 1;
    d.inode = inode;
    for(i = 0; i < 32; i++){

        if(i < strlen(name))
            d.filename[i] = name[i];
        else
            d.filename[i] = '\0';

    }

    dentry_ext_t temp;
    char * dbuf = get_allocated_block();
    if(dbuf == NULL) {
        set_inode_bitmap(inode, 0);
        return -1;
    }
    read_block(INODE_0_LOCATION + dir_inode, dbuf);

    for(i = 2; i < 23; i++){

        temp = *(((dentry_ext_t *) dbuf) + i);
        if(temp.present == 0){

            *(((dentry_ext_t *) dbuf) + i) = d;
            char * ibuf = get_allocated_block();
            if(ibuf == NULL) {
                set_inode_bitmap(inode, 0);
                free_allocated_block(dbuf);
                return -1;
            }
            *(((uint32_t *) ibuf)) = 0;
            *(((uint32_t *) ibuf) + 1) = 0;
            write_block(INODE_0_LOCATION + inode, ibuf);
            write_block(INODE_0_LOCATION + dir_inode, dbuf);
            free_allocated_block(ibuf);
            free_allocated_block(dbuf);
            return 0;

        }

    }

    set_inode_bitmap(inode, 0);
    free_allocated_block(dbuf);
    return -1;

}


/* make an empty file with name in char array at directory specified by
 * directory inode */
int make_empty_file_ext(char * name, uint32_t dir_inode){

    int inode = find_empty_inode();
    if(inode == -1) return -1;

    dentry_ext_t gar;
    if(0 == find_dentry_ext(name, &gar, dir_inode)){
        set_inode_bitmap(inode, 0);
        return -1;
    }

    dentry_ext_t d;
    int i;

    d.filetype = 1;
    d.present = 1;
    d.inode = inode;
    for(i = 0; i < 32; i++){

        if(i < strlen(name))
            d.filename[i] = name[i];
        else
            d.filename[i] = '\0';

    }

    dentry_ext_t temp;
    char * dbuf = get_allocated_block();
    if(dbuf == NULL){
        set_inode_bitmap(inode, 0);
        return -1;
    }
    read_block(INODE_0_LOCATION + dir_inode, dbuf);

    for(i = 2; i < 23; i++){

        temp = *(((dentry_ext_t *) dbuf) + i);
        if(temp.present == 0){

            *(((dentry_ext_t *) dbuf) + i) = d;
            char * ibuf = get_allocated_block();
            if(ibuf == NULL) {
                set_inode_bitmap(inode, 0);
                free_allocated_block(dbuf);
                return -1;
            }
            *(((uint32_t *) ibuf)) = 0;
            *(((uint32_t *) ibuf) + 1) = 0;
            write_block(INODE_0_LOCATION + inode, ibuf);
            write_block(INODE_0_LOCATION + dir_inode, dbuf);
            free_allocated_block(ibuf);
            free_allocated_block(dbuf);
            return 0;

        }

    }

    set_inode_bitmap(inode, 0);
    free_allocated_block(dbuf);
    return -1;

}

/* read directory at index given by idx at a particular dir_inode */
int read_directory(uint32_t idx, char * name, uint32_t dir_inode){

    if(idx >= 23) return -1;
    char * buffer = get_allocated_block();
    read_block(INODE_0_LOCATION + dir_inode, buffer);
    dentry_ext_t d;
    d = *(((dentry_ext_t *) buffer) + idx);
    if(d.present == 0) {
        free_allocated_block(buffer);
        return -1;
    }
    else{
        int i;
        for(i = 0; i < 32; i++){
            name[i] = d.filename[i];
        }
        if(d.filetype == 0){
            free_allocated_block(buffer);
            return 1;
        }
        else{
            free_allocated_block(buffer);
            return 0;
        }

    }

}

/* list current directory by reading curdir using read_directory function */
void list_directory_ext(uint32_t dir_inode){

    char buf[32];
    int i;
    for(i = 0; i < 23; i++){

        int ret = read_directory(i, buf, dir_inode);
        if(ret == -1) continue;
        write_terminal(buf, 32);
        if(ret == 1) printft("/");
        printft("\n");

    }

}

/* find dentry given at dir_inode given by name */
int find_dentry_ext(char * name, dentry_ext_t * dentry, uint32_t dir_inode){

    // if(strlen(name) > 32) return -1;

    char * dbuf = get_allocated_block();
    if(dbuf == NULL) return -1;
    read_block(INODE_0_LOCATION + dir_inode, dbuf);
    int i;
    for(i = 0; i < 23; i++){

        dentry_ext_t d = *(((dentry_ext_t *) dbuf) + i);
        if(0 != strncmp(d.filename, name, 32)) continue;
        if(d.present == 1){
            *(dentry) = d;
            free_allocated_block(dbuf);
            return 0;
        }

    }

    free_allocated_block(dbuf);
    return -1;

}


/* make a new directory at location specified by dir_inode */
int make_directory(char * buffer, uint32_t dir_inode){

    /* no empty inodes */
    int inode = find_empty_inode();
    if(inode == -1) return -1;

    /* dentry already exists at directory */
    dentry_ext_t gar;
    if(0 == find_dentry_ext(buffer, &gar, dir_inode)){
        set_inode_bitmap(inode, 0);
        return -1;
    }

    dentry_ext_t d;
    int i;

    /* filetype = 0 for directory */
    d.filetype = 0;
    d.present = 1;
    d.inode = inode;

    /* write name in filename */
    for(i = 0; i < 32; i++){

        if(i < strlen(buffer))
            d.filename[i] = buffer[i];
        else
            d.filename[i] = '\0';

    }

    /* read block at inode */
    dentry_ext_t temp;
    char * dbuf = get_allocated_block();
    if(dbuf == NULL){
        set_inode_bitmap(inode, 0);
        return -1;
    }
    read_block(INODE_0_LOCATION + dir_inode, dbuf);

    /* loop through dentries at inode */
    for(i = 2; i < 23; i++){

        /* get directory */
        temp = *(((dentry_ext_t *) dbuf) + i);
        /* empty directory */
        if(temp.present == 0){

            /* set inode map, i.e make the new directory */
            *(((dentry_ext_t *) dbuf) + i) = d;
            /* clean the directory values */
            char * ibuf = get_allocated_block();
            if(ibuf == NULL){
                free_allocated_block(dbuf);
                set_inode_bitmap(inode, 0);
                return -1;
            }
            int j;
            for(j = 0; j < 23; j++){

                /* make current directory index */
                dentry_ext_t den;
                if(j == 0){
                    int k;
                    den.filename[0] = '.';
                    for(k = 1; k < 32; k++){
                        den.filename[k] = '\0';
                    }
                    den.filetype = 0;
                    den.inode = inode;
                    den.present = 1;
                    *(((dentry_ext_t *) ibuf) + j) = den;
                }
                /* make parent directory index */
                else if(j == 1){
                    int k;
                    den.filename[0] = '.';
                    den.filename[1] = '.';
                    for(k = 2; k < 32; k++){
                        den.filename[k] = '\0';
                    }
                    den.filetype = 0;
                    den.inode = dir_inode;
                    den.present = 1;
                    *(((dentry_ext_t *) ibuf) + j) = den;
                }
                /* all the other entries are not present */
                else{
                    den.present = 0;
                    *(((dentry_ext_t *) ibuf) + j) = den;
                }
            }
            free_allocated_block(ibuf);
            free_allocated_block(dbuf);
            /* write new directory inode back */
            write_block(INODE_0_LOCATION + inode, ibuf);
            /* write parent directory inode back */
            write_block(INODE_0_LOCATION + dir_inode, dbuf);
            return 0;

        }

    }
    free_allocated_block(dbuf);
    return -1;

}

/* got the be changed */
int set_current_directory(char * buffer){

    char * dbuf = get_allocated_block();
    if(dbuf == NULL) return -1;
    read_block(INODE_0_LOCATION + curdir, dbuf);
    int i;
    for(i = 0; i < 23; i++){

        dentry_ext_t d = *(((dentry_ext_t *) dbuf) + i);
        if(0 != strncmp(d.filename, buffer, strlen(buffer))) continue;
        if(d.present == 1 && d.filetype == 0){
            curdir = d.inode;
            free_allocated_block(dbuf);
            return 0;
        }

    }
    free_allocated_block(dbuf);
    return -1;

}

// void append_fs(){

//     int id;
//     for(id = 0; id < NUM_DIR_ENTRIES; id++){

//         dentry_t d;
//         read_dentry_by_index(id, &d);

//         if(strncmp(d.filename, "fish", 4) == 0){

//             make_empty_file_ext(d.filename, 5);
//             uint32_t length = read_length(d.inode_num);
//             dentry_ext_t den;
//             find_dentry_ext(d.filename, &den, 5);
//             uint32_t counter = 0;
//             while(length > 0){

//                 char data[1024];
//                 uint32_t read = read_data(d.inode_num, 1024*counter, data, 1024);
//                 write_data_ext(den.inode, (uint8_t * )data, 1024*counter, read);
//                 length -= read;
//                 counter++;

//             }

//         }
//         else{

//             make_empty_file_ext(d.filename, 4);
//             uint32_t length = read_length(d.inode_num);
//             dentry_ext_t den;
//             find_dentry_ext(d.filename, &den, 4);
//             uint32_t counter = 0;
//             while(length > 0){

//                 char data[1024];
//                 uint32_t read = read_data(d.inode_num, 1024*counter, data, 1024);
//                 write_data_ext(den.inode, (uint8_t * )data, 1024*counter, read);
//                 length -= read;
//                 counter++;

//             }

//         }

//     }

// }

void reset_fs(){

    char buffer[1024];

    *(((uint32_t *) buffer)) = 1024;
    *(((uint32_t *) buffer) + 1) = 1024*1024;    //NUM_BLOCKS_EXT;
    *(((uint32_t *) buffer) + 2) = 8*1024;       //NUM_INODES_EXT;
    *(((uint32_t *) buffer) + 3) = 1016*1024;    //NUM_DATA_BLOCKS_EXT;
    *(((uint32_t *) buffer) + 4) = 1;            //INODE_BITMAP_LOCATION;
    *(((uint32_t *) buffer) + 5) = 2;            //DATA_BITMAP_LOCATION;
    *(((uint32_t *) buffer) + 6) = 129;          //INODE_0_LOCATION;
    *(((uint32_t *) buffer) + 7) = 129 + 8*1024; //DATA_0_LOCATION;
    *(((uint32_t *) buffer) + 8) = 1016*1024;    //NUM_FREE_DATA_BLOCKS;

    write_block(0, buffer);

    mount();

    int i;
    char zbuf[1024];
    for(i = 0; i < 1024; i++){

        zbuf[i] = 0x00;

    }
    for(i = 1; i < 129; i++){

        write_block(i, zbuf);

    }

    char cd[32];
    char cd2[32];
    cd[0] = '.';
    cd2[0] = '.';
    cd[1] = '\0';
    cd2[1] = '.';

    int id;

    for(id = 2; id < 32; id++){

        cd[id] = '\0';
        cd2[id] = '\0';

    }

    for(id = 0; id < 32; id++){

        buffer[id] = cd[id];
        buffer[64 + id] = cd2[id];

    }

    dentry_ext_t d1;
    dentry_ext_t d2;
    for(id = 0; id < 32; id++){

        d1.filename[id] = cd[id];
        d2.filename[id] = cd2[id];

    }

    d1.inode = 0;
    d1.present = 1;
    d1.filetype = 0;
    d2.inode = 0;
    d2.present = 1;
    d2.filetype = 0;

    *(((dentry_ext_t *) buffer)) = d1;
    *(((dentry_ext_t *) buffer) + 1) = d2;

    dentry_ext_t ed;
    ed.present = 0;
    for(id = 2; id < 23; id++){
        *(((dentry_ext_t *) buffer) + id) = ed;
    }

    set_inode_bitmap(0, 1);
    write_block(INODE_0_LOCATION, buffer);

    // make utils folder
    char * name;
    name = "utils";
    make_directory(name, 0);

    name = "text";
    make_directory(name, 0);

    name = "dev";
    make_directory(name, 0);

    name = "progs"; // 4
    make_directory(name, 0);

    name = "fish"; // 5
    make_directory(name, 4);

    name = "tests"; // 6
    make_directory(name, 0);

    name = "gui"; // 7
    make_directory(name, 0);

    name = "signals"; // 8
    make_directory(name, 0);

    name = "assets"; // 9
    make_directory(name, 7);

    name = "music"; // 10
    make_directory(name, 0);

    //list_directory_ext(0);
}

void append_fs(){

    int id;
    for(id = 0; id < NUM_DIR_ENTRIES; id++){

        dentry_t d;
        read_dentry_by_index(id, &d);


        if(strncmp(d.filename, "terminal.txt", 7) == 0 || strncmp(d.filename, "mouse.txt", 7) == 0
            || strncmp(d.filename, "bg.txt", 5) == 0 || strncmp(d.filename, "terminalicon.txt", 12) == 0
            || strncmp(d.filename, "fishicon.txt", 7) == 0 || strncmp(d.filename, "pingpongicon.txt", 14) == 0
            || strncmp(d.filename, "countericon.txt", 10) == 0 || strncmp(d.filename, "status.txt", 8) == 0
            || strncmp(d.filename, "50x50.txt", 7) == 0 || strncmp(d.filename, "100x50.txt", 7) == 0
            || strncmp(d.filename, "25x25.txt", 7) == 0 || strncmp(d.filename, "50x25.txt", 7) == 0){

            make_empty_file_ext(d.filename, 9);
            uint32_t length = read_length(d.inode_num);
            dentry_ext_t den;
            find_dentry_ext(d.filename, &den, 9);
            uint32_t counter = 0;
            while(length > 0){

                char data[1024];
                uint32_t read = read_data(d.inode_num, 1024*counter, data, 1024);
                write_data_ext(den.inode, (uint8_t * )data, 1024*counter, read);
                length -= read;
                counter++;

            }

        }
        else if(strncmp(d.filename, "guilink", 7) == 0){

            make_empty_file_ext(d.filename, 7);
            uint32_t length = read_length(d.inode_num);
            dentry_ext_t den;
            find_dentry_ext(d.filename, &den, 7);
            uint32_t counter = 0;
            while(length > 0){

                char data[1024];
                uint32_t read = read_data(d.inode_num, 1024*counter, data, 1024);
                write_data_ext(den.inode, (uint8_t * )data, 1024*counter, read);
                length -= read;
                counter++;

            }

        }
        else if(strncmp(d.filename, "verylargetextwithverylongname.tx", 32) == 0 || strncmp(d.filename,"created.txt",7) == 0){

            make_empty_file_ext(d.filename, 2);
            uint32_t length = read_length(d.inode_num);
            dentry_ext_t den;
            find_dentry_ext(d.filename, &den, 2);
            uint32_t counter = 0;
            while(length > 0){

                char data[1024];
                uint32_t read = read_data(d.inode_num, 1024*counter, data, 1024);
                write_data_ext(den.inode, (uint8_t * )data, 1024*counter, read);
                length -= read;
                counter++;

            }

        }
        else if(strncmp(d.filename, "shell", 5) == 0 || strncmp(d.filename, "grep", 4) == 0
                || strncmp(d.filename, "ls", 2) == 0 || strncmp(d.filename, "cat", 3) == 0
                || strncmp(d.filename, "cd", 2) == 0 || strncmp(d.filename, "touch", 5) == 0
                || strncmp(d.filename, "mkdir", 5) == 0 || strncmp(d.filename, "rm", 2) == 0
                || strncmp(d.filename, "rmdir", 5) == 0 || strncmp(d.filename, "textedit", 7) == 0){

            make_empty_file_ext(d.filename, 1);
            uint32_t length = read_length(d.inode_num);
            dentry_ext_t den;
            find_dentry_ext(d.filename, &den, 1);
            uint32_t counter = 0;
            while(length > 0){

                char data[1024];
                uint32_t read = read_data(d.inode_num, 1024*counter, data, 1024);
                write_data_ext(den.inode, (uint8_t * )data, 1024*counter, read);
                length -= read;
                counter++;

            }

        }
        else if(strncmp(d.filename, "syserr", 5) == 0 || strncmp(d.filename, "sigtest", 5) == 0
                || strncmp(d.filename, "exception", 8) == 0){

            make_empty_file_ext(d.filename, 6);
            uint32_t length = read_length(d.inode_num);
            dentry_ext_t den;
            find_dentry_ext(d.filename, &den, 6);
            uint32_t counter = 0;
            while(length > 0){

                char data[1024];
                uint32_t read = read_data(d.inode_num, 1024*counter, data, 1024);
                write_data_ext(den.inode, (uint8_t * )data, 1024*counter, read);
                length -= read;
                counter++;

            }
        }
        else if(strncmp(d.filename, "pingpong", 5) == 0 || strncmp(d.filename, "hello", 5) == 0
                || strncmp(d.filename, "counter", 5) == 0 || strncmp(d.filename, "testprint", 8) == 0
                || strncmp(d.filename, "blink", 5) == 0 || strncmp(d.filename, "guitest", 7) == 0
                || strncmp(d.filename, "testsound", 8) == 0) {

            make_empty_file_ext(d.filename, 4);
            uint32_t length = read_length(d.inode_num);
            dentry_ext_t den;
            find_dentry_ext(d.filename, &den, 4);
            uint32_t counter = 0;
            while(length > 0){

                char data[1024];
                uint32_t read = read_data(d.inode_num, 1024*counter, data, 1024);
                write_data_ext(den.inode, (uint8_t * )data, 1024*counter, read);
                length -= read;
                counter++;

            }
        }
        else if(strncmp(d.filename, "fish", 4) == 0 || strncmp(d.filename, "frame0.txt", 7) == 0|| strncmp(d.filename, "frame1.txt", 7) == 0){

            make_empty_file_ext(d.filename, 5);
            uint32_t length = read_length(d.inode_num);
            dentry_ext_t den;
            find_dentry_ext(d.filename, &den, 5);
            uint32_t counter = 0;
            while(length > 0){

                char data[1024];
                uint32_t read = read_data(d.inode_num, 1024*counter, data, 1024);
                write_data_ext(den.inode, (uint8_t * )data, 1024*counter, read);
                length -= read;
                counter++;

            }
        }
        else if(strncmp(d.filename, "rtc", 3) == 0){

            make_device_file_ext(d.filename, 3);

        }
        else if(strncmp(d.filename, "siglink", 7) == 0 || strncmp(d.filename, "testignore", 9) == 0){

            make_empty_file_ext(d.filename, 8);
            uint32_t length = read_length(d.inode_num);
            dentry_ext_t den;
            find_dentry_ext(d.filename, &den, 8);
            uint32_t counter = 0;
            while(length > 0){

                char data[1024];
                uint32_t read = read_data(d.inode_num, 1024*counter, data, 1024);
                write_data_ext(den.inode, (uint8_t * )data, 1024*counter, read);
                length -= read;
                counter++;

            }

        }
        else if(strncmp(d.filename, "allthestars.wav", 7) == 0){

            make_empty_file_ext(d.filename, 10);
            uint32_t length = read_length(d.inode_num);
            dentry_ext_t den;
            find_dentry_ext(d.filename, &den, 10);
            uint32_t counter = 0;
            while(length > 0){

                char data[1024];
                uint32_t read = read_data(d.inode_num, 1024*counter, data, 1024);
                write_data_ext(den.inode, (uint8_t * )data, 1024*counter, read);
                length -= read;
                counter++;

            }

        }
        else{

            make_empty_file_ext(d.filename, 0);
            uint32_t length = read_length(d.inode_num);
            dentry_ext_t den;
            find_dentry_ext(d.filename, &den, 0);
            uint32_t counter = 0;
            while(length > 0){

                char data[1024];
                uint32_t read = read_data(d.inode_num, 1024*counter, data, 1024);
                write_data_ext(den.inode, (uint8_t * )data, 1024*counter, read);
                length -= read;
                counter++;

            }

        }

    }

    make_device_file_ext("gui", 3);
    make_device_file_ext("sb16", 3);

}

uint32_t read_complete_block(uint32_t inode, uint32_t offset, uint8_t * buf, uint32_t length){

    uint32_t block_to_read = offset/1024;
    uint32_t read_counter;

    if(block_to_read < 252){

        read_counter = block_to_read + 2;

    }
    else if(block_to_read < 252 + 256){

        read_counter = 254;

    }
    else{

        read_counter = 255;

    }

    if(read_counter == 254){

        char * file_buf = get_allocated_block();
        if(file_buf == NULL) return 0;
        read_block(INODE_0_LOCATION + inode, file_buf);
        uint32_t block_idx = *(((uint32_t *) file_buf) + read_counter);

        char * inner_data_buf = get_allocated_block();
        if(inner_data_buf == NULL){
            free_allocated_block(file_buf);
            return 0;
        }
        read_block(DATA_0_LOCATION + block_idx, inner_data_buf);
        uint32_t inner_block_idx = block_to_read - 252;
        block_idx = *(((uint32_t *) inner_data_buf) + inner_block_idx);

        read_block(DATA_0_LOCATION + block_idx, (char *)buf);

        free_allocated_block(file_buf);
        free_allocated_block(inner_data_buf);
        return 1024;

    }
    else if(read_counter == 255){

        char * file_buf = get_allocated_block();
        if(file_buf == 0) {return 0;}
        read_block(INODE_0_LOCATION + inode, file_buf);
        uint32_t block_idx = *(((uint32_t *) file_buf) + read_counter);

        char * inner_data_buf = get_allocated_block();
        if(inner_data_buf == NULL){
            free_allocated_block(file_buf);
            return 0;
        }
        read_block(DATA_0_LOCATION + block_idx, inner_data_buf);
        uint32_t inner_block_idx = (block_to_read - 252 - 256)/256;
        block_idx = *(((uint32_t *) inner_data_buf) + inner_block_idx);

        read_block(DATA_0_LOCATION + block_idx, inner_data_buf);
        inner_block_idx = (block_to_read - 252 - 256)%256;
        block_idx = *(((uint32_t *) inner_data_buf) + inner_block_idx);

        read_block(DATA_0_LOCATION + block_idx, buf);

        free_allocated_block(file_buf);
        free_allocated_block(inner_block_idx);
        return 1024;


    }
    else{

        char * file_buf = get_allocated_block();
        if(file_buf == 0) return 0;
        read_block(INODE_0_LOCATION + inode, file_buf);
        uint32_t block_idx = *(((uint32_t *) file_buf) + read_counter);

        read_block(DATA_0_LOCATION + block_idx, (char *)buf);
        free_allocated_block(file_buf);
        return 1024;

    }

}

uint32_t read_partial_block(uint32_t inode, uint32_t offset, uint8_t * buf, uint32_t length){

    uint32_t block_to_read = offset/1024;
    uint32_t read_counter;

    if(block_to_read < 252){

        read_counter = block_to_read + 2;

    }
    else if(block_to_read < 252 + 256){

        read_counter = 254;

    }
    else{

        read_counter = 255;

    }

    if(read_counter == 254){

        char * file_buf = get_allocated_block();
        if(file_buf == NULL) return 0;
        read_block(INODE_0_LOCATION + inode, file_buf);
        uint32_t block_idx = *(((uint32_t *) file_buf) + read_counter);

        char * inner_data_buf = get_allocated_block();
        if(inner_data_buf == NULL){
            free_allocated_block(file_buf);
            return 0;
        }
        read_block(DATA_0_LOCATION + block_idx, inner_data_buf);
        uint32_t inner_block_idx = block_to_read - 252;
        block_idx = *(((uint32_t *) inner_data_buf) + inner_block_idx);

        char * data_buf = get_allocated_block();
        if(data_buf == NULL){
            free_allocated_block(file_buf);
            free_allocated_block(inner_data_buf);
            return 0;
        }
        read_block(DATA_0_LOCATION + block_idx, data_buf);

        uint32_t data_off = offset%1024;

        uint32_t possible_read = 1024 - data_off;
        if(possible_read > length) possible_read = length;

        uint32_t i;
        uint32_t buf_pos = 0;

        for(i = 0; i < possible_read; i++){

            buf[buf_pos++] = data_buf[data_off++];

        }

        free_allocated_block(file_buf);
        free_allocated_block(inner_data_buf);
        free_allocated_block(data_buf);
        return possible_read;

    }
    else if(read_counter == 255){

        char * file_buf = get_allocated_block();
        if(file_buf == NULL) return 0;
        read_block(INODE_0_LOCATION + inode, file_buf);
        uint32_t block_idx = *(((uint32_t *) file_buf) + read_counter);

        char * inner_data_buf = get_allocated_block();
        if(inner_data_buf == NULL){
            free_allocated_block(file_buf);
            return 0;
        }
        read_block(DATA_0_LOCATION + block_idx, inner_data_buf);
        uint32_t inner_block_idx = (block_to_read - 252 - 256)/256;
        block_idx = *(((uint32_t *) inner_data_buf) + inner_block_idx);

        read_block(DATA_0_LOCATION + block_idx, inner_data_buf);
        inner_block_idx = (block_to_read - 252 - 256)%256;
        block_idx = *(((uint32_t *) inner_data_buf) + inner_block_idx);

        char * data_buf = get_allocated_block();
        if(data_buf == NULL){
            free_allocated_block(file_buf);
            free_allocated_block(inner_data_buf);
            return 0;
        }
        read_block(DATA_0_LOCATION + block_idx, data_buf);

        uint32_t data_off = offset%1024;

        uint32_t possible_read = 1024 - data_off;
        if(possible_read > length) possible_read = length;

        uint32_t i;
        uint32_t buf_pos = 0;

        for(i = 0; i < possible_read; i++){

            buf[buf_pos++] = data_buf[data_off++];

        }

        free_allocated_block(file_buf);
        free_allocated_block(inner_data_buf);
        free_allocated_block(data_buf);
        return possible_read;


    }
    else{

        char * file_buf = get_allocated_block();
        if(file_buf == NULL) return 0;
        read_block(INODE_0_LOCATION + inode, file_buf);
        uint32_t block_idx = *(((uint32_t *) file_buf) + read_counter);

        char * data_buf = get_allocated_block();
        if(data_buf == NULL){
            free_allocated_block(file_buf);
            return 0;
        }
        read_block(DATA_0_LOCATION + block_idx, data_buf);
        uint32_t data_off = offset%1024;

        uint32_t possible_read = 1024 - data_off;
        if(possible_read > length) possible_read = length;

        uint32_t i;
        uint32_t buf_pos = 0;

        for(i = 0; i < possible_read; i++){

            buf[buf_pos++] = data_buf[data_off++];

        }

        free_allocated_block(file_buf);
        free_allocated_block(data_buf);
        return possible_read;

    }

}

uint32_t read_partial_block_end(uint32_t inode, uint32_t offset, uint8_t * buf, uint32_t length){

    uint32_t block_to_read = offset/1024;
    uint32_t read_counter;

    if(block_to_read < 252){

        read_counter = block_to_read + 2;

    }
    else if(block_to_read < 252 + 256){

        read_counter = 254;

    }
    else{

        read_counter = 255;

    }

    if(read_counter == 254){

        char * file_buf = get_allocated_block();
        if(file_buf == NULL) return 0;
        read_block(INODE_0_LOCATION + inode, file_buf);
        uint32_t block_idx = *(((uint32_t *) file_buf) + read_counter);

        char * inner_data_buf = get_allocated_block();
        if(inner_data_buf == NULL){
            free_allocated_block(file_buf);
            return 0;
        }
        read_block(DATA_0_LOCATION + block_idx, inner_data_buf);
        uint32_t inner_block_idx = block_to_read - 252;
        block_idx = *(((uint32_t *) inner_data_buf) + inner_block_idx);

        char * data_buf = get_allocated_block();
        if(data_buf == NULL){
            free_allocated_block(file_buf);
            free_allocated_block(inner_data_buf);
            return 0;
        }
        read_block(DATA_0_LOCATION + block_idx, data_buf);
        memcpy(buf, data_buf, length);

        free_allocated_block(file_buf);
        free_allocated_block(inner_data_buf);
        free_allocated_block(data_buf);
        return length;

    }
    else if(read_counter == 255){

        char * file_buf = get_allocated_block();
        if(file_buf == NULL) return 0;
        read_block(INODE_0_LOCATION + inode, file_buf);
        uint32_t block_idx = *(((uint32_t *) file_buf) + read_counter);

        char * inner_data_buf = get_allocated_block();
        if(inner_data_buf == NULL){
            free_allocated_block(file_buf);
            return 0;
        }
        read_block(DATA_0_LOCATION + block_idx, inner_data_buf);
        uint32_t inner_block_idx = (block_to_read - 252 - 256)/256;
        block_idx = *(((uint32_t *) inner_data_buf) + inner_block_idx);

        read_block(DATA_0_LOCATION + block_idx, inner_data_buf);
        inner_block_idx = (block_to_read - 252 - 256)%256;
        block_idx = *(((uint32_t *) inner_data_buf) + inner_block_idx);

        char * data_buf = get_allocated_block();
        if(data_buf == NULL){
            free_allocated_block(file_buf);
            free_allocated_block(inner_data_buf);
            return 0;
        }
        read_block(DATA_0_LOCATION + block_idx, data_buf);
        memcpy(buf, data_buf, length);

        free_allocated_block(file_buf);
        free_allocated_block(inner_data_buf);
        free_allocated_block(data_buf);
        return length;

    }
    else{

        char * file_buf = get_allocated_block();
        if(file_buf == NULL) return 0;
        read_block(INODE_0_LOCATION + inode, file_buf);
        uint32_t block_idx = *(((uint32_t *) file_buf) + read_counter);

        char * data_buf = get_allocated_block();
        if(data_buf == NULL){
            free_allocated_block(file_buf);
            return 0;
        }
        read_block(DATA_0_LOCATION + block_idx, data_buf);
        memcpy(buf, data_buf, length);

        free_allocated_block(file_buf);
        free_allocated_block(data_buf);
        return length;

    }

}

/*
 * read_data_ext
 * DESCRIPTION -- Reads data from a particular inode starting from input
 *                offset, and reading length number of bytes into the buf
 * INPUTS -- inode - Inode num to get data from
 *           offset - offset to start reading from
 *           buf -- pointer to data array to store data in
 *           length -- number of bytes to read
 * OUTPUTS -- None
 * Return Value -- Number of bytes read, or -1 on invalid inode number
 * Side Effects -- Changes input Buffer
 */
int32_t read_data_ext(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {

    char * inode_buf = get_allocated_block();
    if(inode_buf == NULL) return -1;
    read_block(INODE_0_LOCATION + inode, inode_buf);

    uint32_t file_flags = *(((uint32_t *) inode_buf));

    unsigned long flags;
    cli_and_save(flags);

    // if(file_flags & WRITE_LOCK){
    //     free_allocated_block(inode_buf);
    //     restore_flags(flags);
    //     return -2;
    // }
    // else{

    //     file_flags = file_flags | READ_LOCK;
    //     *(((uint32_t *) inode_buf)) = file_flags;
    //     write_block(INODE_0_LOCATION + inode, inode_buf);

    // }

    restore_flags(flags);


    read_block(INODE_0_LOCATION + inode, inode_buf);

    uint32_t cur_length = *(((uint32_t *) inode_buf) + 1);

    if(offset > cur_length) return 0;

    uint32_t length_to_read = length;

    if(offset + length > cur_length){

        length_to_read = cur_length - offset;

    }

    if(length_to_read == 0) return 0;

    uint32_t return_val = length_to_read;
    uint32_t offset_read = 0;
    uint32_t cur_offset = offset;
    uint32_t next_block_idx = offset/1024 + 2;

    if(offset%1024 > 0){

        uint32_t ret = read_partial_block(inode, offset, buf, length_to_read);
        if(ret == 0) {
            free_allocated_block(inode_buf);
            return -1;
        }
        offset_read += ret;
        length_to_read -= offset_read;
        cur_offset += offset_read;
        next_block_idx++;

    }

    // write complete blocks
    while(length_to_read/1024 > 0){

        uint32_t ret = read_complete_block(inode, cur_offset, buf + offset_read, 1024);
        if(ret == 0){
            free_allocated_block(inode_buf);
            return -1;
        }
        offset_read += 1024;
        cur_offset += 1024;
        length_to_read -= 1024;
        // uint32_t read_counter;
        // if(next_block_idx <= 253){

        //     read_counter = next_block_idx;

        // }
        // else if(next_block_idx <= 253 + )

        // if(next_block_idx == 254){

        //     char inode_buf2[1024];
        //     read_block(INODE_0_LOCATION + inode, inode_buf2);
        //     uint32_t block_idx = *(((uint32_t *) inode_buf2) + next_block_idx);

        // }

        // char inode_buf2[1024];
        // read_block(INODE_0_LOCATION + inode, inode_buf2);
        // uint32_t block_idx = *(((uint32_t *) inode_buf2) + next_block_idx);

        // char block[1024];
        // read_block(DATA_0_LOCATION + block_idx, block);
        // memcpy(buf + offset_read, block, 1024);
        // length_to_read -= 1024;
        // offset_read += 1024;
        // next_block_idx++;

    }

    if(length_to_read > 0){

        // char inode_buf2[1024];
        // read_block(INODE_0_LOCATION + inode, inode_buf2);
        // uint32_t block_idx = *(((uint32_t *) inode_buf2) + next_block_idx);

        // char block[1024];
        // read_block(DATA_0_LOCATION + block_idx, block);
        // memcpy(buf + offset_read, block, length_to_read);
        uint32_t ret = read_partial_block_end(inode, cur_offset, buf + offset_read, length_to_read);
        if(ret == 0){
            free_allocated_block(inode_buf);
            return -1;
        }

    }

    read_block(INODE_0_LOCATION + inode, inode_buf);
    file_flags = *(((uint32_t *) inode_buf));

    cli_and_save(flags);

    file_flags = file_flags & ~READ_LOCK;
    *(((uint32_t *) inode_buf)) = file_flags;
    write_block(INODE_0_LOCATION + inode, inode_buf);

    restore_flags(flags);

    free_allocated_block(inode_buf);
    return return_val;

}

uint32_t write_complete_block(uint32_t inode, uint8_t * buf, uint32_t offset, uint32_t length, uint32_t curlength){

    uint32_t block_to_write = offset/1024;
    uint32_t write_counter;

    if(block_to_write < 252){

        write_counter = block_to_write + 2;

    }
    else if(block_to_write < 252 + 256){

        write_counter = 254;

    }
    else{

        write_counter = 255;

    }

    if(write_counter == 254){

        int alloc;

        if(curlength <= 252 * 1024 && offset == 252 * 1024){

            alloc = find_empty_datablock();
            set_data_bitmap(alloc, 1);
            lastfreeblock = alloc;
            char alloc_block[1024];
            read_block(INODE_0_LOCATION + inode, alloc_block);
            *(((uint32_t *) alloc_block) + write_counter) = alloc;
            write_block(INODE_0_LOCATION + inode, alloc_block);

        }

        char file_buf[1024];
        read_block(INODE_0_LOCATION + inode, file_buf);
        uint32_t block_idx = *(((uint32_t *) file_buf) + write_counter);

        if(offset >= curlength){

            alloc = find_empty_datablock();
            set_data_bitmap(alloc, 1);
            lastfreeblock = alloc;
            char alloc_block[1024];
            read_block(DATA_0_LOCATION + block_idx, alloc_block);
            uint32_t inner_block_idx = block_to_write - 252;
            *(((uint32_t *) alloc_block) + inner_block_idx) = alloc;
            write_block(DATA_0_LOCATION + block_idx, alloc_block);

        }

        char inner_data_buf[1024];
        read_block(DATA_0_LOCATION + block_idx, inner_data_buf);
        uint32_t inner_block_idx = block_to_write - 252;
        block_idx = *(((uint32_t *) inner_data_buf) + inner_block_idx);

        char data_buf[1024];
        read_block(DATA_0_LOCATION + block_idx, data_buf);
        uint32_t data_off = offset%1024;
        uint32_t possible_write = 1024 - data_off;
        if(possible_write > length) possible_write = length;
        uint32_t i;
        uint32_t buf_pos = 0;

        for(i = 0; i < possible_write; i++){

            data_buf[data_off++] = buf[buf_pos++];

        }
        write_block(DATA_0_LOCATION + block_idx, data_buf);
        return possible_write;

    }
    else if(write_counter == 255){

        int alloc;

        if(curlength <= 252 * 1024 + 256 * 1024 && offset == 252 * 1024 + 256 * 1024){

            alloc = find_empty_datablock();
            set_data_bitmap(alloc, 1);
            lastfreeblock = alloc;
            char alloc_block[1024];
            read_block(INODE_0_LOCATION + inode, alloc_block);
            *(((uint32_t *) alloc_block) + write_counter) = alloc;
            write_block(INODE_0_LOCATION + inode, alloc_block);

        }

        char file_buf[1024];
        read_block(INODE_0_LOCATION + inode, file_buf);
        uint32_t block_idx = *(((uint32_t *) file_buf) + write_counter);

        uint32_t inner_block_idx = (block_to_write - 252 - 256)/256;

        if(curlength <= 252 * 1024 + 256 * 1024 + inner_block_idx * 256 * 1024 && offset == 252 * 1024 + 256 * 1024 + inner_block_idx * 256 * 1024){

            alloc = find_empty_datablock();
            set_data_bitmap(alloc, 1);
            lastfreeblock = alloc;
            char alloc_block[1024];
            read_block(DATA_0_LOCATION + block_idx, alloc_block);
            *(((uint32_t *) alloc_block) + inner_block_idx) = alloc;
            write_block(DATA_0_LOCATION + block_idx, alloc_block);

        }

        char inner_data_buf[1024];
        read_block(DATA_0_LOCATION + block_idx, inner_data_buf);
        inner_block_idx = (block_to_write - 252 - 256)/256;
        block_idx = *(((uint32_t *) inner_data_buf) + inner_block_idx);

        inner_block_idx = (block_to_write - 252 - 256)%256;

        if(offset >= curlength){

            alloc = find_empty_datablock();
            set_data_bitmap(alloc, 1);
            lastfreeblock = alloc;
            char alloc_block[1024];
            read_block(DATA_0_LOCATION + block_idx, alloc_block);
            *(((uint32_t *) alloc_block) + inner_block_idx) = alloc;
            write_block(DATA_0_LOCATION + block_idx, alloc_block);

        }

        read_block(DATA_0_LOCATION + block_idx, inner_data_buf);
        inner_block_idx = (block_to_write - 252 - 256)%256;
        block_idx = *(((uint32_t *) inner_data_buf) + inner_block_idx);

        char data_buf[1024];
        read_block(DATA_0_LOCATION + block_idx, data_buf);
        uint32_t data_off = offset%1024;
        uint32_t possible_write = 1024 - data_off;
        if(possible_write > length) possible_write = length;
        uint32_t i;
        uint32_t buf_pos = 0;

        for(i = 0; i < possible_write; i++){

            data_buf[data_off++] = buf[buf_pos++];

        }
        write_block(DATA_0_LOCATION + block_idx, data_buf);
        return possible_write;

    }
    else{

        int alloc;
        if(curlength <= block_to_write * 1024){

            alloc = find_empty_datablock();
            set_data_bitmap(alloc, 1);
            lastfreeblock = alloc;
            char alloc_block[1024];
            read_block(INODE_0_LOCATION + inode, alloc_block);
            *(((uint32_t *) alloc_block) + write_counter) = alloc;
            write_block(INODE_0_LOCATION + inode, alloc_block);

        }

        char file_buf[1024];
        read_block(INODE_0_LOCATION + inode, file_buf);
        uint32_t block_idx = *(((uint32_t *) file_buf) + write_counter);

        char data_buf[1024];
        read_block(DATA_0_LOCATION + block_idx, data_buf);
        uint32_t data_off = offset%1024;
        uint32_t possible_write = 1024 - data_off;
        if(possible_write > length) possible_write = length;
        uint32_t i;
        uint32_t buf_pos = 0;

        for(i = 0; i < possible_write; i++){

            data_buf[data_off++] = buf[buf_pos++];

        }
        write_block(DATA_0_LOCATION + block_idx, data_buf);
        return possible_write;
    }

}

uint32_t write_partial_block(uint32_t inode, uint8_t * buf, uint32_t offset, uint32_t length){

    uint32_t block_to_write = offset/1024;
    uint32_t write_counter;

    if(block_to_write < 252){

        write_counter = block_to_write + 2;

    }
    else if(block_to_write < 252 + 256){

        write_counter = 254;

    }
    else{

        write_counter = 255;

    }

    if(write_counter == 254){

        char file_buf[1024];
        read_block(INODE_0_LOCATION + inode, file_buf);
        uint32_t block_idx = *(((uint32_t *) file_buf) + write_counter);

        char inner_data_buf[1024];
        read_block(DATA_0_LOCATION + block_idx, inner_data_buf);
        uint32_t inner_block_idx = block_to_write - 252;
        block_idx = *(((uint32_t *) file_buf) + inner_block_idx);

        char data_buf[1024];
        read_block(DATA_0_LOCATION + block_idx, data_buf);
        // int i;
        // for(i = 0; i < length; i++){

        //     data_buf[i] = buf[i];

        // }
        // write_block(DATA_0_LOCATION + block_idx, data_buf);
        // return length;
        uint32_t data_off = offset%1024;

        uint32_t possible_write = 1024 - data_off;
        if(possible_write > length) possible_write = length;

        uint32_t i;
        uint32_t buf_pos = 0;

        for(i = 0; i < possible_write; i++){

            data_buf[data_off++] = buf[buf_pos++];

        }

        write_block(DATA_0_LOCATION + block_idx, data_buf);
        return possible_write;

    }
    else if(write_counter == 255){

        char file_buf[1024];
        read_block(INODE_0_LOCATION + inode, file_buf);
        uint32_t block_idx = *(((uint32_t *) file_buf) + write_counter);

        char inner_data_buf[1024];
        read_block(DATA_0_LOCATION + block_idx, inner_data_buf);
        uint32_t inner_block_idx = (block_to_write - 252 - 256)/256;
        block_idx = *(((uint32_t *) file_buf) + inner_block_idx);

        read_block(DATA_0_LOCATION + block_idx, inner_data_buf);
        inner_block_idx = (block_to_write - 252 - 256)%256;
        block_idx = *(((uint32_t *) file_buf) + inner_block_idx);

        char data_buf[1024];
        read_block(DATA_0_LOCATION + block_idx, data_buf);
        // int i;
        // for(i = 0; i < length; i++){

        //     data_buf[i] = buf[i];

        // }
        // write_block(DATA_0_LOCATION + block_idx, data_buf);
        // return length;

        uint32_t data_off = offset%1024;

        uint32_t possible_write = 1024 - data_off;
        if(possible_write > length) possible_write = length;

        uint32_t i;
        uint32_t buf_pos = 0;

        for(i = 0; i < possible_write; i++){

            data_buf[data_off++] = buf[buf_pos++];

        }

        write_block(DATA_0_LOCATION + block_idx, data_buf);
        return possible_write;

    }
    else{

        char file_buf[1024];
        read_block(INODE_0_LOCATION + inode, file_buf);
        uint32_t block_idx = *(((uint32_t *) file_buf) + write_counter);

        char data_buf[1024];
        read_block(DATA_0_LOCATION + block_idx, data_buf);
        // int i;
        // for(i = 0; i < length; i++){

        //     data_buf[i] = buf[i];

        // }
        // write_block(DATA_0_LOCATION + block_idx, data_buf);
        // return length;
        uint32_t data_off = offset%1024;

        uint32_t possible_write = 1024 - data_off;
        if(possible_write > length) possible_write = length;

        uint32_t i;
        uint32_t buf_pos = 0;

        for(i = 0; i < possible_write; i++){

            data_buf[data_off++] = buf[buf_pos++];

        }

        write_block(DATA_0_LOCATION + block_idx, data_buf);
        return possible_write;
    }

}

uint32_t write_partial_block_end(uint32_t inode, uint8_t * buf, uint32_t offset, uint32_t length, uint32_t curlength){

    uint32_t block_to_write = offset/1024;
    uint32_t write_counter;

    if(block_to_write < 252){

        write_counter = block_to_write + 2;

    }
    else if(block_to_write < 252 + 256){

        write_counter = 254;

    }
    else{

        write_counter = 255;

    }

    if(write_counter == 254){

        int alloc;

        if(curlength <= 252 * 1024 && offset == 252 * 1024){

            alloc = find_empty_datablock();
            set_data_bitmap(alloc, 1);
            lastfreeblock = alloc;
            char alloc_block[1024];
            read_block(INODE_0_LOCATION + inode, alloc_block);
            *(((uint32_t *) alloc_block) + write_counter) = alloc;
            write_block(INODE_0_LOCATION + inode, alloc_block);

        }

        char file_buf[1024];
        read_block(INODE_0_LOCATION + inode, file_buf);
        uint32_t block_idx = *(((uint32_t *) file_buf) + write_counter);

        if(offset >= curlength){

            alloc = find_empty_datablock();
            set_data_bitmap(alloc, 1);
            lastfreeblock = alloc;
            char alloc_block[1024];
            read_block(DATA_0_LOCATION + block_idx, alloc_block);
            uint32_t inner_block_idx = block_to_write - 252;
            *(((uint32_t *) alloc_block) + inner_block_idx) = alloc;
            write_block(DATA_0_LOCATION + block_idx, alloc_block);

        }

        char inner_data_buf[1024];
        read_block(DATA_0_LOCATION + block_idx, inner_data_buf);
        uint32_t inner_block_idx = block_to_write - 252;
        block_idx = *(((uint32_t *) inner_data_buf) + inner_block_idx);

        char data_buf[1024];
        read_block(DATA_0_LOCATION + block_idx, data_buf);

        int i;
        for(i = 0; i < length; i++){

            data_buf[i] = buf[i];

        }
        write_block(DATA_0_LOCATION + block_idx, data_buf);
        return length;

    }
    else if(write_counter == 255){

        int alloc;

        if(curlength <= 252 * 1024 + 256 * 1024 && offset == 252 * 1024 + 256 * 1024){

            alloc = find_empty_datablock();
            set_data_bitmap(alloc, 1);
            lastfreeblock = alloc;
            char alloc_block[1024];
            read_block(INODE_0_LOCATION + inode, alloc_block);
            *(((uint32_t *) alloc_block) + write_counter) = alloc;
            write_block(INODE_0_LOCATION + inode, alloc_block);

        }

        char file_buf[1024];
        read_block(INODE_0_LOCATION + inode, file_buf);
        uint32_t block_idx = *(((uint32_t *) file_buf) + write_counter);

        uint32_t inner_block_idx = (block_to_write - 252 - 256)/256;

        if(curlength <= 252 * 1024 + 256 * 1024 + inner_block_idx * 256 * 1024 && offset == 252 * 1024 + 256 * 1024 + inner_block_idx * 256 * 1024){

            alloc = find_empty_datablock();
            set_data_bitmap(alloc, 1);
            lastfreeblock = alloc;
            char alloc_block[1024];
            read_block(DATA_0_LOCATION + block_idx, alloc_block);
            *(((uint32_t *) alloc_block) + inner_block_idx) = alloc;
            write_block(DATA_0_LOCATION + block_idx, alloc_block);

        }

        char inner_data_buf[1024];
        read_block(DATA_0_LOCATION + block_idx, inner_data_buf);
        inner_block_idx = (block_to_write - 252 - 256)/256;
        block_idx = *(((uint32_t *) inner_data_buf) + inner_block_idx);

        inner_block_idx = (block_to_write - 252 - 256)%256;

        if(offset >= curlength){

            alloc = find_empty_datablock();
            set_data_bitmap(alloc, 1);
            lastfreeblock = alloc;
            char alloc_block[1024];
            read_block(DATA_0_LOCATION + block_idx, alloc_block);
            *(((uint32_t *) alloc_block) + inner_block_idx) = alloc;
            write_block(DATA_0_LOCATION + block_idx, alloc_block);

        }

        read_block(DATA_0_LOCATION + block_idx, inner_data_buf);
        inner_block_idx = (block_to_write - 252 - 256)%256;
        block_idx = *(((uint32_t *) inner_data_buf) + inner_block_idx);

        char data_buf[1024];
        read_block(DATA_0_LOCATION + block_idx, data_buf);
        int i;
        for(i = 0; i < length; i++){

            data_buf[i] = buf[i];

        }
        write_block(DATA_0_LOCATION + block_idx, data_buf);
        return length;

    }
    else{

        int alloc;
        if(curlength <= write_counter * 1024){

            alloc = find_empty_datablock();
            set_data_bitmap(alloc, 1);
            lastfreeblock = alloc;
            char alloc_block[1024];
            read_block(INODE_0_LOCATION + inode, alloc_block);
            *(((uint32_t *) alloc_block) + write_counter) = alloc;
            write_block(INODE_0_LOCATION + inode, alloc_block);

        }

        char file_buf[1024];
        read_block(INODE_0_LOCATION + inode, file_buf);
        uint32_t block_idx = *(((uint32_t *) file_buf) + write_counter);

        char data_buf[1024];
        read_block(DATA_0_LOCATION + block_idx, data_buf);
        int i;
        for(i = 0; i < length; i++){

            data_buf[i] = buf[i];

        }
        write_block(DATA_0_LOCATION + block_idx, data_buf);
        return length;
    }

}

int32_t write_data_ext(uint32_t inode, uint8_t * buf, uint32_t offset, uint32_t length){

    char inode_buf[1024];
    read_block(INODE_0_LOCATION + inode, inode_buf);

    uint32_t file_flags = *(((uint32_t *) inode_buf));

    // unsigned long flags;
    // cli_and_save(flags);

    // if(file_flags & WRITE_LOCK || file_flags & READ_LOCK) return -2;
    // else{

    //     file_flags = file_flags | WRITE_LOCK;
    //     *(((uint32_t *) inode_buf)) = file_flags;
    //     write_block(INODE_0_LOCATION + inode, inode_buf);

    // }

    // restore_flags(flags);

    uint32_t cur_length = *(((uint32_t *) inode_buf) + 1);

    if(offset > cur_length) return -1;

    uint32_t new_length = cur_length;

    uint32_t cur_blocks = cur_length/1024;
    if(cur_blocks%1024 > 0) cur_blocks++;

    if(offset + length > cur_length){ // have to allocate new blocks

        new_length = offset + length;
        // *(((uint32_t *) inode_buf) + 1) = offset + length;
        // uint32_t last_block_length = cur_length%1024;
        // if(last_block_length == 0) num_of_blocks_to_alloc++;
        // uint32_t length_to_add = offset + length - cur_length;
        // if((last_block_length + length_to_add) > 1024){

        //     uint32_t remaining_length = last_block_length + length_to_add - 1024;
        //     num_of_blocks_to_alloc += remaining_length/1024;
        //     if(remaining_length%1024 > 0) num_of_blocks_to_alloc++;

        // }
    }

    *(((uint32_t *) inode_buf) + 1) = new_length;
    write_block(INODE_0_LOCATION + inode, inode_buf);

    uint32_t length_to_write = length;
    uint32_t offset_written = 0;
    uint32_t next_block_idx = offset/1024 + 2;
    uint32_t curoffset = offset;

    if(offset%1024 > 0){

        offset_written += write_partial_block(inode, buf, offset, length);
        length_to_write -= offset_written;
        next_block_idx++;
        curoffset += offset_written;

    }

    // write complete blocks
    while(length_to_write/1024 > 0){

        write_complete_block(inode, buf + offset_written, curoffset, 1024 ,cur_length);
        curoffset += 1024;
        length_to_write -= 1024;
        offset_written += 1024;

        // if((next_block_idx-1)*1024 < cur_length){ // no need to allocate new blocks

        //     char inode_buf2[1024];
        //     read_block(INODE_0_LOCATION + inode, inode_buf2);
        //     uint32_t block_idx = *(((uint32_t *) inode_buf2) + next_block_idx);

        //     char block[1024];
        //     read_block(DATA_0_LOCATION + block_idx, block);
        //     memcpy(block, buf + offset_written, 1024);
        //     write_block(DATA_0_LOCATION + block_idx, block);
        //     length_to_write -= 1024;
        //     offset_written += 1024;
        //     next_block_idx++;

        // }
        // else{ // allocate a new block

        //     char inode_buf2[1024];
        //     read_block(INODE_0_LOCATION + inode, inode_buf2);

        //     int free_block = find_empty_datablock();
        //     set_data_bitmap(free_block, 1);

        //     char block[1024];
        //     memcpy(block, buf + offset_written, 1024);
        //     write_block(DATA_0_LOCATION + free_block, block);
        //     length_to_write -= 1024;
        //     offset_written += 1024;

        //     *(((uint32_t *) inode_buf2) + next_block_idx) = free_block;
        //     // printft("allocated block %d at index %d\n", free_block, next_block_idx);
        //     write_block(INODE_0_LOCATION + inode, inode_buf2);

        //     next_block_idx++;

        // }

    }

    // if ending partial, write ending partial

    if(length_to_write > 0){

        write_partial_block_end(inode, buf + offset_written, curoffset, length_to_write, cur_length);

        // if((next_block_idx-1)*1024 < cur_length){ // no need to allocate new blocks

        //     char inode_buf2[1024];
        //     read_block(INODE_0_LOCATION + inode, inode_buf2);
        //     uint32_t block_idx = *(((uint32_t *) inode_buf2) + next_block_idx);

        //     char block[1024];
        //     read_block(DATA_0_LOCATION + block_idx, block);
        //     memcpy(block, buf + offset_written, length_to_write);
        //     write_block(DATA_0_LOCATION + block_idx, block);

        // }
        // else{ // allocate a new block

        //     char inode_buf2[1024];
        //     read_block(INODE_0_LOCATION + inode, inode_buf2);

        //     int free_block = find_empty_datablock();
        //     set_data_bitmap(free_block, 1);

        //     char block[1024];
        //     memcpy(block, buf + offset_written, length_to_write);
        //     write_block(DATA_0_LOCATION + free_block, block);

        //     *(((uint32_t *) inode_buf2) + next_block_idx) = free_block;
        //     // printft("allocated block %d at index %d\n", free_block, next_block_idx);
        //     write_block(INODE_0_LOCATION + inode, inode_buf2);
        // }

    }

    read_block(INODE_0_LOCATION + inode, inode_buf);
    file_flags = *(((uint32_t *) inode_buf));

    // cli_and_save(flags);

    // file_flags = file_flags & ~WRITE_LOCK;
    // *(((uint32_t *) inode_buf)) = file_flags;

    // write_block(INODE_0_LOCATION + inode, inode_buf);
    // restore_flags(flags);

    return length;

    // uint32_t first_block_to_write = offset/1024 + 1;
    // uint32_t write_counter = 0;

    // if(first_block_to_write <= 252){

    //     write_counter = first_block_to_write + 2;

    // }
    // else if(first_block_to_write <= 252 + 256){

    //     write_counter = 254;

    // }
    // else{

    //     write_counter = 255;

    // }



    // return num_of_blocks_to_alloc;

    // uint32_t blocks_written = 0;

    // uint32_t write_counter = 2;
    // uint32_t blocks_allocated = 0;
    // uint32_t bytes_written = 0;
    // uint32_t current_pos = 0;

    // while(bytes_written < length){

    //     int free_block = find_empty_datablock();
    //     set_data_bitmap((uint32_t)free_block, 1);

    //     if(write_counter == 254){ //indirect block

    //         *(((uint32_t *) inode_buf) + write_counter) = (uint32_t)(free_block);
    //         char inner_buffer[1024];

    //         int k = 0;
    //         while( (k < 256) && (i < blocks)){

    //             char data[1024];
    //             memcpy((void *)data, (void *)(buf + i*1024) , 1024);
    //             int free_block_inner = find_empty_datablock();
    //             write_block(DATA_0_LOCATION + free_block_inner, data);
    //             *(((uint32_t *) inner_buffer) + k) = (uint32_t) free_block_inner;
    //             set_data_bitmap((uint32_t)free_block_inner, 1);

    //             k++;
    //             i++;

    //         }

    //         write_block(DATA_0_LOCATION + free_block, inner_buffer);
    //         write_counter++;

    //     }
    //     else if(write_counter == 255){ // double indirect block

    //         *(((uint32_t *) inode_buf) + write_counter) = (uint32_t)(free_block);
    //         char inner_buffer_1[1024];
    //         int k = 0;

    //         while(k < 256 && i < blocks){

    //             int free_block_inner_1 = find_empty_datablock();
    //             *(((uint32_t *) inner_buffer_1) + k) = (uint32_t) free_block_inner_1;
    //             set_data_bitmap((uint32_t)free_block_inner_1, 1);
    //             char inner_buffer_2[1024];

    //             int l = 0;
    //             while(l < 256 && i < blocks){

    //                 char data[1024];
    //                 memcpy((void *)(data), (void *)(buf + i*1024) , 1024);
    //                 int free_block_inner_2 = find_empty_datablock();
    //                 write_block(DATA_0_LOCATION + free_block_inner_2, data);
    //                 set_data_bitmap((uint32_t)free_block_inner_2, 1);
    //                 *(((uint32_t *) inner_buffer_2) + l) = (uint32_t) free_block_inner_2;

    //                 l++;
    //                 i++;

    //             }

    //             write_block(DATA_0_LOCATION + free_block_inner_1, inner_buffer_2);
    //             k++;

    //         }

    //         write_block(DATA_0_LOCATION + free_block, inner_buffer_1);

    //     }
    //     else{



    //         /* allocate a block */
    //         int free_block = find_empty_datablock();
    //         set_data_bitmap((uint32_t)free_block, 1);

    //         /* block to write in buffer */
    //         char data[1024];
    //         memcpy((void *)data, (void *)(buf + blocks_written*1024) , 1024);
    //         write_block(DATA_0_LOCATION + free_block, data);


    //         *(((uint32_t *) inode_buf) + write_counter) = (uint32_t)(free_block);
    //         write_counter++;

    //     }

    // }

    // write_block(INODE_0_LOCATION + inode, inode_buf);
    // return 0;

}

int delete_file_ext(uint32_t file_inode, uint32_t dir_inode){

    char dir[1024];
    read_block(INODE_0_LOCATION + dir_inode, dir);
    int i;
    dentry_ext_t d;
    int flag = 0;
    for(i = 0; i < 23; i++){

        d = *(((dentry_ext_t *) dir) + i);
        if(d.present == 1 && d.filetype == 1 && d.inode == file_inode){

            d.present = 0;
            *(((dentry_ext_t *) dir) + i) = d;
            write_block(INODE_0_LOCATION + dir_inode, dir);
            flag = 1;
            break;

        }

    }

    /* file not found */
    if(flag == 0) return -1;

    cleanup_file_ext(file_inode);

    set_inode_bitmap(file_inode, 0);
    lastfreeinode = file_inode;

    return 0;

}

/* free all data blocks and change num free data blocks */
void cleanup_file_ext(uint32_t file_inode){

    char dir[1024];
    read_block(INODE_0_LOCATION + file_inode, dir);

    uint32_t length = *(((uint32_t *)dir) + 1);
    *(((uint32_t *)dir) + 1) = 0;
    uint32_t blocks = length/1024;
    if(blocks%1024 > 0) blocks++;

    uint32_t freed_blocks = blocks;

    uint32_t write_counter = 2;
    int i;
    for(i = 0; i < blocks; i++){

        if(write_counter == 254){ //indirect block

            uint32_t indirect_block = *(((uint32_t *) dir) + write_counter);
            char inner_buffer[1024];
            read_block(DATA_0_LOCATION + indirect_block, inner_buffer);

            int k = 0;
            while( (k < 256) && (i < blocks)){

                uint32_t inner_data_block = *(((uint32_t *) inner_buffer) + k);
                set_data_bitmap(inner_data_block, 0);

                k++;
                i++;

            }

            set_data_bitmap(indirect_block, 0);
            freed_blocks++;
            write_counter++;

        }
        else if(write_counter == 255){ // double indirect block

            uint32_t double_indirect_block = *(((uint32_t *) dir) + write_counter);
            char inner_buffer_1[1024];
            read_block(DATA_0_LOCATION + double_indirect_block, inner_buffer_1);
            set_data_bitmap(double_indirect_block, 0);
            freed_blocks++;
            int k = 0;

            while(k < 256 && i < blocks){

                uint32_t indirect_block = *(((uint32_t *) inner_buffer_1) + k);
                char inner_buffer_2[1024];
                read_block(DATA_0_LOCATION + indirect_block, inner_buffer_2);
                set_data_bitmap((uint32_t)indirect_block, 0);
                freed_blocks++;

                int l = 0;
                while(l < 256 && i < blocks){

                    uint32_t alloc_block = *(((uint32_t *) inner_buffer_2) + l);
                    set_data_bitmap(alloc_block, 0);

                    l++;
                    i++;

                }

                k++;

            }

        }
        else{

            uint32_t alloc_block = *(((uint32_t *) dir) + write_counter);
            set_data_bitmap(alloc_block, 0);
            write_counter++;

        }

    }

    // uint32_t free_blocks = get_free_blocks();
    // free_blocks -= freed_blocks;
    // set_free_blocks(free_blocks);

    write_block(INODE_0_LOCATION + file_inode, dir);
    return;

}

/* parse the path */
int parse_path(char * path, uint32_t dir_inode, char * name){

    if(path[0] == '/'){

        int i = 1;
        uint32_t start_dir = 0;
        uint32_t buf_idx = 0;
        char buffer[32];
        reset_buffer(buffer);
        while(i < strlen(path)){

            if(path[i] == '/'){

                dentry_ext_t d;
                int ret = find_dentry_ext(buffer, &d, start_dir);
                if(d.filetype != 0 || ret == -1) return -1; // not directory
                start_dir = d.inode;
                reset_buffer(buffer);
                buf_idx = 0;

            }
            else{

                buffer[buf_idx] = path[i];
                buf_idx++;

            }

            i++;

        }

        for(i = 0; i < 32; i++){

            name[i] = buffer[i];

        }

        return start_dir;

    }
    else{

        int i = 0;
        uint32_t start_dir = dir_inode;
        uint32_t buf_idx = 0;
        char buffer[32];
        reset_buffer(buffer);
        while(i < strlen(path)){

            if(path[i] == '/'){

                dentry_ext_t d;
                int ret = find_dentry_ext(buffer, &d, start_dir);
                if(d.filetype != 0 || ret == -1) return -1; // not directory
                start_dir = d.inode;
                reset_buffer(buffer);
                buf_idx = 0;

            }
            else{

                buffer[buf_idx] = path[i];
                buf_idx++;

            }

            i++;

        }

        for(i = 0; i < 32; i++){

            name[i] = buffer[i];

        }

        return start_dir;

    }

}

/* reset buffer */
void reset_buffer(char * buffer){

    int i;
    for(i = 0; i < 32; i++) buffer[i] = '\0';

}

uint32_t get_free_blocks(){

    unsigned long flags;
    cli_and_save(flags);
    char zbuf[1024];
    read_block(0, zbuf);
    uint32_t free_blocks = *(((uint32_t *) zbuf) + 8);
    restore_flags(flags);
    return free_blocks;


}

void set_free_blocks(uint32_t new_free_blocks){

    unsigned long flags;
    cli_and_save(flags);
    char zbuf[1024];
    read_block(0, zbuf);
    *(((uint32_t *) zbuf) + 8) = new_free_blocks;
    write_block(0, zbuf);
    restore_flags(flags);

}

int read_dentry_by_index_ext(uint32_t index, dentry_ext_t * d, uint32_t dir_num){

    if(index >= 23) return -1;
    char dir_buf[1024];
    read_block(INODE_0_LOCATION + dir_num, dir_buf);

    uint32_t counter = 0;
    int i;
    for(i = 0; i < 23; i++){

        *d = *(((dentry_ext_t *) dir_buf) + i);
        if(d->present == 1 && counter == index) return 0;
        else if(d->present == 1) counter++;

    }
    return -1;

}

uint32_t read_length_ext(uint32_t inode){

    char buf[1024];
    read_block(INODE_0_LOCATION + inode, buf);
    uint32_t length = *(((uint32_t *) buf) + 1);
    return length;

}

int delete_dir(uint32_t dir_num){

    // read_block(INODE_0_LOCATION + dir_num, buf);
    char buf[1024];
    // remove from parent dir
    dentry_ext_t parent;
    find_dentry_ext("..", &parent, dir_num);
    uint32_t parent_inode = parent.inode;
    read_block(INODE_0_LOCATION + parent_inode, buf);

    int i;
    for(i = 2; i < 23; i++){

        read_dentry_by_index_ext(i, &parent, parent_inode);
        if(parent.inode == dir_num && parent.present == 1){ // found

            parent.present = 0; // remove dir
            *(((dentry_ext_t *) buf) + i) = parent;
            write_block(INODE_0_LOCATION + parent_inode, buf);
            break;

        }

    }

    // delete files
    for(i = 2; i < 23; i++){

        read_dentry_by_index_ext(i, &parent, dir_num);
        if(parent.present == 1) { // delete this file

            int ret = delete_file_ext(parent.inode, dir_num);
            if(ret == -1) return -1;

        }

    }

    // reclaim dir inode
    set_inode_bitmap(dir_num, 0);
    lastfreeinode = dir_num;
    return 0;

}
