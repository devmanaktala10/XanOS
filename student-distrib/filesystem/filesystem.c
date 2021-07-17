#include "filesystem.h"

// global variable to track directory entries
uint32_t cur_dir_index = 0;

/*
 * init_fs
 * DESCRIPTION -- Initializes the file system by saving base pointer,
 *                Number of Directories, inodes, and data blocks
 * INPUTS -- fs_start - Pointer to the start of the filesystem in memory
 * OUTPUTS -- None
 * RETURN VALUE -- None
 * Side Effects -- Changes fundamental file system variables
 */
void init_fs(uint32_t * fs_start){

    // fs_base now points to start of file system
    fs_base = fs_start;

    // First 4 Bytes store number of directories
    NUM_DIR_ENTRIES = *(fs_start);

    // Second 4 Bytes store number of inodes
    NUM_INODES = *(fs_start + INODE_OFF);

    // Third 4 Bytes store number of Data blocks
    NUM_DATA_BLOCKS = *(fs_start + DATA_OFF);

}

/*
 * read_dentry_by_name
 * DESCRIPTION -- Through the input fname provided, this function finds the
 *                relevant dentry and makes the input pointer dentry point to
 *                that directory entry in the filesystem
 * INPUTS -- fname - Name of the dentry to find
 *           dentry - Must point to the required dentry on return
 * OUTPUTS -- None
 * Return Value -- 0 on successfuly finding dentry, and -1 if there is
 *                 no dentry in the system with that particular fname
 * Side Effects -- None
 */
int32_t read_dentry_by_name(const uint8_t * fname, dentry_t * dentry){

    /* check for NULL pointers */
    if(fname == NULL || dentry == NULL){
      return -1;
    }

    // cur_dir now points to start of Directory Entries
    uint32_t * cur_dir = fs_base + DENTRY_BLOCK;

    // Temporary variable to store the dentry to compare
    dentry_t cur_dentry;

    int i;

    /* Loops through directory entries and compares their names to find the
        relevant dentry */
    for(i = 0; i < NUM_DIR_ENTRIES; i++){
        memcpy(&cur_dentry, cur_dir, sizeof(dentry_t));

        // If dentry name is correct, make input pointer, point to that location
        if(strncmp(cur_dentry.filename, (char*) fname, MAX_FILENAME_LENGTH) == 0){
            *(dentry) = cur_dentry;
            return 0;
        }
        // go to next memory block
        cur_dir += DENTRY_BLOCK;
    }

    // return -1 on failure
    return -1;

}

/*
 * get_dentry_index_by_name
 * DESCRIPTION -- Through the input fname provided, this function finds the
 *                relevant dentry and makes the input pointer dentry point to
 *                that directory entry in the filesystem and returns the index
 *                of the dentry
 * INPUTS -- fname - Name of the dentry to find
 *           dentry - Must point to the required dentry on return
 * OUTPUTS -- None
 * Return Value -- index of dentry, and -1 if there is
 *                 no dentry in the system with that particular fname
 * Side Effects -- None
 */
int32_t get_dentry_index_by_name(const uint8_t * fname, dentry_t * dentry){

    /* check for NULL pointers */
    if(fname == NULL || dentry == NULL){
      return -1;
    }

    // cur_dir now points to start of Directory Entries
    uint32_t * cur_dir = fs_base + DENTRY_BLOCK;

    // Temporary variable to store the dentry to compare
    dentry_t cur_dentry;

    int i;

    /* Loops through directory entries and compares their names to find the
        relevant dentry */
    for(i = 0; i < NUM_DIR_ENTRIES; i++){
        memcpy(&cur_dentry, cur_dir, sizeof(dentry_t));

        // If dentry name is correct, make input pointer, point to that location
        if(strncmp(cur_dentry.filename, (char*) fname, MAX_FILENAME_LENGTH) == 0){
            *(dentry) = cur_dentry;
            return i;
        }
        // go to next memory block
        cur_dir += DENTRY_BLOCK;
    }

    // return -1 on failure
    return -1;

}

/*
 * read_dentry_by_index
 * DESCRIPTION -- Through the input index provided, this function finds the
 *                relevant dentry  at that index and makes the input pointer
 *                dentry point to that directory entry in the filesystem
 * INPUTS -- index - Index of the dentry to get
 *           dentry - Must point to the required dentry on return
 * OUTPUTS -- None
 * Return Value -- 0 on successfuly finding dentry, and -1 if the index
 *                 is invalid
 * Side Effects -- None
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry) {

  /* check for NULL pointers */
  if(dentry == NULL){
    return -1;
  }

  // Return -1 if index is invalid
  if (index >= NUM_DIR_ENTRIES || index < 0) {
    return -1;
  }

  // cur_dir now points to start of directory entries
  uint32_t* cur_dir = fs_base + DENTRY_BLOCK;

  // Get to the relevant Directry entry
  cur_dir += index * DENTRY_BLOCK;

  // Copy the block to dentry input pointer and return 0
  memcpy(dentry, cur_dir, sizeof(dentry_t));
  return 0;

}

/*
 * read_data
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
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {

  /* check for NULL pointers */
  if (buf == NULL) {
    return -1;
  }

  if (inode >= NUM_INODES) {
    return -1;
  }

  // All calculations based on inode starting from 0

  uint32_t* rel_inode;
  uint8_t* rel_block;
  int32_t i;

  rel_inode = fs_base + (inode + 1) * (SIZE_OF_BLOCK / 4);

  // If length required is greater than the length of the file - offset,
  // I make length return the entire file from that point
  if (*rel_inode - offset < length) {
    length = *rel_inode - offset;
  }

  // rel_inode now points to data blocks
  rel_inode++;

  // rel_inode now points to block to start reading from
  rel_inode += (offset / SIZE_OF_BLOCK);

  // Offset in First Block
  int32_t offset_in_first_block = offset % SIZE_OF_BLOCK;

  // rel block stores the block to get data from
  rel_block = (uint8_t*) (fs_base + ((NUM_INODES + (*rel_inode) + 1) * (SIZE_OF_BLOCK / 4)));

  // bytes_to_read corresponds to amount of data to be read in first block
  uint32_t bytes_to_read = SIZE_OF_BLOCK - offset_in_first_block;

  // Gets first data block
  if (bytes_to_read >= length) {
    memcpy(buf, rel_block + offset_in_first_block, length);
    return length;
  }
  memcpy(buf, rel_block + offset_in_first_block, bytes_to_read);

  // Num of blocks to read after start
  int32_t blocks_to_read = (int32_t) ((length - bytes_to_read) / SIZE_OF_BLOCK);
  uint32_t buf_num = bytes_to_read;

  for (i = 0; i < blocks_to_read; i++) {
      rel_inode++; // go to next data block
      // go to starting point of relevant block in memory
      rel_block = (uint8_t*) (fs_base + ((NUM_INODES + (*rel_inode) + 1) * (SIZE_OF_BLOCK / 4)));
      // copies entire block
      memcpy(buf + buf_num, rel_block, SIZE_OF_BLOCK);
      buf_num += SIZE_OF_BLOCK;
  }

  // TODO fix blocks_to_read

  // Read data from last block
  uint32_t bytes_left_to_read = length - (bytes_to_read +
            blocks_to_read * SIZE_OF_BLOCK);

  if (bytes_left_to_read != 0) {
    rel_inode++;
    rel_block = (uint8_t*) (fs_base + ((NUM_INODES + (*rel_inode) + 1) * (SIZE_OF_BLOCK / 4)));
    memcpy(buf + buf_num, rel_block, bytes_left_to_read);
  }

  // Return Number of bytes read
  return length;

}

/*
 * read_length
 * DESCRIPTION -- Gets number of Bytes of data stored in an inode
 * INPUTS -- inode - Inode num to get data from
 * OUTPUTS -- None
 * Return Value -- Number of bytes read, or -1 on invalid inode number
 * Side Effects -- Changes input Buffer
 */
uint32_t read_length(uint32_t inode){

  // return -1 on invalid inode number
  if (inode >= NUM_INODES) {
    return -1;
  }

  // Get the relative inode and return number of bytes stored
  uint32_t* rel_inode;
  rel_inode = fs_base + (inode + 1) * (SIZE_OF_BLOCK / 4);
  return *rel_inode;

}

/*
 * file_open
 * DESCRIPTION -- Opens file with the given filename and starts tracking it
 * INPUTS -- filename - file to open
 * OUTPUTS -- None
 * Return Value -- file_index to access file from, or -1 for failure
 * Side Effects -- Changes global file tracker variable file potentially
 */
int32_t file_open() {

  // Not really needed
  return 0;

}

/*
 * file_close
 * DESCRIPTION -- closes file with the given file_index and clears space
 * INPUTS -- file_index - file to close
 * OUTPUTS -- None
 * Return Value -- 0 for successful file close, -1 otherwise
 * Side Effects -- closes files and clears space in file_tracker array
 */
int32_t file_close() {
  // Currently Does Nothing
  return 0;
}

/*
 * file_read
 * DESCRIPTION -- reads data from an opened file, taking offset into account
 * INPUTS -- file_index -- file to read from
 *           buf -- buffer to put data in
 *`          length -- Number of bytes to read`
 * OUTPUTS -- None
 * Return Value -- Number of bytes read, or -1 for invalid inputs
 * Side Effects -- reads data from a file, and makes offset go ahead
 */
int32_t file_read(void* buf, int32_t length) {

  /* check for NULL pointer */
  if(buf == NULL){
    return -1;
  }

  file_t* rel_file = &(process_pcb->file[rel_fd]);

  // Call read_data to get data from file
  int32_t bytes_read = read_data_ext(rel_file->inode, rel_file->fileposition,
                                    (uint8_t*) buf, length);

  // Invalid case
  if (bytes_read < 0) {
    return -1;
  }

  // Add bytes_read to cur_offset for tracking position to read from
  rel_file->fileposition += (uint32_t) bytes_read;

  // return no. of bytes read
  return bytes_read;

}

/*
 * file_write
 * DESCRIPTION -- Does nothing, writing to file not supported
 * INPUTS -- file_index - file to write to
 * OUTPUTS -- None
 * Return Value -- Always -1 for failure
 * Side Effects -- None
 */
int32_t file_write(const void* buf, int32_t length) {
  /* check for NULL pointer */
  if(buf == NULL){
    return -1;
  }

  file_t* rel_file = &(process_pcb->file[rel_fd]);

  return write_data_ext(rel_file->inode, buf, 0, length);

}

/*
 * dir_open
 * DESCRIPTION -- Tries to open a directory in the current filesystem
 * INPUTS -- dirname -- name of directory to open
 * OUTPUTS -- None
 * Return Value -- 0 for successful open, -1 otherwise
 * Side Effects -- None
 */
int32_t dir_open() {
  // Not really needed
  return 0;
}

int32_t dir_ioctl(unsigned long cmd, unsigned long arg) {
  return -1;
}

int32_t file_ioctl(unsigned long cmd, unsigned long arg){
  return -1;
}

/*
 * dir_close
 * DESCRIPTION -- Does nothing, dir_close is meaningless
 * INPUTS -- None
 * OUTPUTS -- None
 * Return Value -- Always 0
 * Side Effects -- None
 */
int32_t dir_close() {
  // Not really needed
  return 0;
}

/*
 * dir_close
 * DESCRIPTION -- Does nothing, dir_write is meaningless
 * INPUTS -- None
 * OUTPUTS -- None
 * Return Value -- Always -1
 * Side Effects -- None
 */
int32_t dir_write(const void* buf, int32_t length) {
  return -1;
}

/*
 * dir_read
 * DESCRIPTION -- reads dentry data in order, tracked through cur_dir_index
 * INPUTS -- dentry to write data to
 * OUTPUTS -- None
 * Return Value -- -1 if end of dentry, otherwise 0
 * Side Effects -- None
 */
int32_t dir_read(void* buf, int32_t length) {

  file_t* rel_file = &(process_pcb->file[rel_fd]);

  // Reached end of directory
  if (rel_file->fileposition >= 23) {
    return 0;
  }

  dentry_ext_t dentry;
  // Get dentry tracked through rel_file->fileposition
  //read_dentry_by_index(rel_file->fileposition, &dentry);
  if(-1 == read_dentry_by_index_ext(rel_file->fileposition, &dentry , rel_file->inode)) return 0;
  rel_file->fileposition++;

  if(dentry.filetype == 0){
    dentry.filename[strlen(dentry.filename)] = '/';
  }

  uint32_t retval = strlen(dentry.filename);

  if (retval > 32) {
    retval = 32;
  }

  // copy filename into buffer
  memcpy((char*) buf, dentry.filename, retval);

  // return no. of characters read
  return retval;
}
