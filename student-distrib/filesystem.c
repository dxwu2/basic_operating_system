/* filesystem.c - sets up filesystem, includes some other functions regarding fs */

#include "filesystem.h"
#include "lib.h"

/* local static for boot block */
static boot_block_t* the_boot_block = NULL;

/* File system initialization
 * Only one module loaded in c, so it must be filesystem
 * Call init_file_system() from kernel.c using mod captured from multiboot info mbi
 * Inputs:  fs_start - start address of filesystem module
 *          fs_end - end address of filesystem module
 * Outputs: returns 0 on success, -1 on failure (bits of module lost or field sizes don't add up)
 */
uint32_t init_file_system(uint32_t fs_start, uint32_t fs_end){
    boot_block_t* potential_boot_block = (boot_block_t*) fs_start;     //fs_start is start addr of file system (boot block)

    /* Verify structure of fs using fs_start, fs_end, and absolute block count
     * (just checking to make sure we are actually reading valid data structure and not some random bits)
     * Count of absolute blocks = 1 (the boot block) + number of index nodes + number of data blocks */
    uint32_t abs_block_count = 1 + potential_boot_block->inode_count + potential_boot_block->data_block_count;

    /* Multiply by 4kB per block and compare against fs_end*/
    uint32_t fs_size = 4096 * abs_block_count;
    if (fs_start + fs_size != fs_end)
        return -1;
    /* Maybe more checks?? */

    else{
        filesystem_start = (uint32_t) potential_boot_block;
        the_boot_block = potential_boot_block;      //officially set local boot block to fs_start
    }
    return 0;
}

/* Helper functions defined */

/* read_dentry_by_name - fills a dentry block with the file name, file type, and inode number for the file
 * Inputs   : fname - filename
 *          : dentry - ptr to the dentry block we want to fill
 * Outputs  : returns 0 on success, -1 on failure (non-existent file or invalid index)
 */ 
uint32_t read_dentry_by_name (const int8_t* fname, dentry_t* dentry){
    unsigned i;     //for loop below
    /* Check if filesystem initialized */
    if(!the_boot_block)
        return -1;
    /* Check inputs for validity */
    if(!fname || !dentry)
        return -1;
    /* If fname length greater than 32, truncate to 32 chars for search */
    int8_t fname_len = (strlen(fname) > FILENAME_LEN) ? FILENAME_LEN : strlen(fname);
    /* scan through dentries in boot block to find fname */
    for(i = 0; i < NUM_DIRENTRIES; i++){
        dentry_t* potential_dentry = &(the_boot_block->direntries[i]);
        if(!strncmp(fname, potential_dentry->filename, fname_len)){
            *dentry = *potential_dentry;
            return 0;
        }
    }
    return -1;      //We could not find dentry with desired filename
}

/* read_dentry_by_index - fills a dentry block with the file name, file type, and inode number for the file
 * Inputs   : index - inode index
 *          : dentry - ptr to the dentry block we want to fill
 * Outputs  : returns 0 on success, -1 on failure (non-existent file or invalid index)
 */
uint32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){
    /* Check if filesystem initialized */
    if(!the_boot_block)
        return -1;
    /* Check inputs for validity (0 <= index < number of dentries)*/
    if((index < 0) || (index >= the_boot_block->dir_entry_count) || (!dentry))
        return -1;
    /* Set dentry based on index otherwise */
    *dentry = the_boot_block->direntries[index];
    return 0;
}

/* read_data - reads up to "length" bytes starting from "offset" position in the file with inode number "inode"
 * Inputs   : inode - the inode index that indicates which file
 *          : offset - tells us where to start reading in the file
 *          : buf - read info placed in here
 *          : length - tell us how many bytes to read
 * Outputs  : returns the number of bytes read, or 0 if end of file reached, or -1 on failure
 * Side effects : bytes read are placed in the buffer
 */
uint32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    /* check if filesystem not initialized or if inode number is invalid */
    if (!the_boot_block || !buf || inode > (the_boot_block->inode_count) - 1) {
        return -1;
    }

    /* retrieve inode */
    uint32_t* i = (uint32_t*)((uint32_t)the_boot_block + BLOCK_SIZE * (inode + 1));

    /* if offset is greater than length of inode(first element of i is its length), then reached end of file */
    if (offset > *i) {
        return 0;
    }

    /* first, get us ptr to start of data blocks (N blocks after boot block) */
    uint32_t* start_datablocks = (uint32_t*)((uint32_t)the_boot_block + BLOCK_SIZE * (the_boot_block->inode_count + 1));
    /* next, get us index of 0th data block # (index) */
    int num_data_block = 0;     // # of data block we are on in the inode
    //uint32_t* cur_datablock_index_addr = (uint32_t*)((inode_t*) i)->data_block_num[num_data_block]; // addr
    uint32_t* cur_datablock_index_addr = i + 1;     // add 1 (uint32_t, so 1 = 4B) to get us address at 0th data block number (in inode)
    // apply offset
    cur_datablock_index_addr += (offset/BLOCK_SIZE) * 4;       // each data block index is 4B (diagram), division is floored in C
    uint32_t cur_datablock_index = *cur_datablock_index_addr;
    num_data_block = (offset/BLOCK_SIZE);
    // get the actual data block's addr
    uint32_t* cur_datablock = start_datablocks + cur_datablock_index * BLOCK_SIZE/sizeof(uint32_t);

    /* setup for looping */
    uint32_t position_in_file = offset % BLOCK_SIZE;

    /* loop through data and read to buffer */
    uint32_t bytes_read;
    for (bytes_read = 0; bytes_read < length; bytes_read += 4) {
        // each value in buffer is only 1B, but we read 4B at a time (in little endian format):
        uint32_t test01 = (*(cur_datablock + position_in_file)) >> 24;              // we want leftmost byte
        uint32_t test02 = ((*(cur_datablock + position_in_file)) >> 16) & 0xFF;     // we want second byte
        uint32_t test03 = ((*(cur_datablock + position_in_file)) >> 8) & 0xFF;      // we want third byte
        uint32_t test04 = ((*(cur_datablock + position_in_file))) & 0xFF;           // we want rightmost byte
        buf[bytes_read] = test04;
        buf[bytes_read + 1] = test03;
        buf[bytes_read + 2] = test02;
        buf[bytes_read + 3] = test01;

        position_in_file++;
        /* if we reach end of current data block */
        // we increment position_in_file only once every 4 bytes
        if (position_in_file >= BLOCK_SIZE/4) {
            position_in_file = 0;   // reset position in file
            num_data_block++;
            cur_datablock_index = (uint32_t)((inode_t*) i)->data_block_num[num_data_block];
            cur_datablock = start_datablocks + cur_datablock_index * BLOCK_SIZE/sizeof(uint32_t);
        }
        /* if we reach EOF (each inode can hold up to 1023 data blocks, and its 0-indexed so 0-1022) */
        if (num_data_block > NUM_DATA_BLOCKS - 1) {
            return 0;
        }
    }
    return bytes_read;
}


/* Local functions defined
 * Define file operation functions separate from directory operations 
 * As stated in Discussion Week 9, return -1 for any write functions 
 */

 /* uint32_t file_open(const uint8_t* filename) - initializes any temporary structures
  * Inputs  : filename
  * Outputs : returns 0 on success, -1 on failure
  * Side Effects    : change global inode number to the one associated with this file (used in file_read)
  */
uint32_t file_open(const uint8_t* filename) {
    uint32_t* cur_dentry;
    int i;
    /* loop through dentries to find the one with matching filename */
    for (i = 1; i <= NUM_DIRENTRIES; i++) {
        cur_dentry = ((uint32_t*)the_boot_block + (sizeof(dentry_t)/sizeof(uint32_t)) * i);
        if (strncmp((int8_t*)filename, ((dentry_t*)cur_dentry)->filename, FILENAME_LEN) == 0) {
            // set global inode number to one associated w/ this file
            global_inode_index = ((dentry_t*)cur_dentry)->inode_num;
            return 0;
        }
    }
    // file not found
    return -1;
}

/* file_close - doesn't do anything for this checkpoint
 * Inputs   : fd - file descriptor (unused here)
 * Outputs  : 0
 */
uint32_t file_close(uint32_t fd){
    return 0;
}

/* uint32_t file_read(uint32_t fd, void* buf, uint32_t nbytes) - reads nbytes of data from file into buf
 * Inputs   : file descriptor (unused here)
 *          : buf - info read will be placed in here
 *          : nbytes - number of bytes 
 * Outputs  : >= 0 on success, -1 on failure (check generic read function header)
 * 
 */ 
uint32_t file_read(uint32_t fd, void* buf, uint32_t nbytes){
    /* use read_data */
    return read_data(global_inode_index, 0, buf, nbytes);   // start at offset 0 because we want the whole file
}

/* file_write - doesn't do anything for this checkpoint
 * Inputs   fd : file descriptor
 *          buf : buffer
 *          nbytes : number of bytes
 * Outputs  : -1
 */
uint32_t file_write(uint32_t fd, void* buf, uint32_t nbytes){
    return -1;      //do nothing, return -1
}


/* dir_open - opens a directory file - initialize temporary structure
 * Inputs   : filename
 * Outputs  : 0
 */
uint32_t dir_open(const int8_t* filename){
    dentry_t* cur_dentry;
    read_dentry_by_name(filename, cur_dentry);
    return 0;
}

/* dir_close - closes a directory file (does nothing this checkpoint)
 * Inputs   : file descriptor
 * Outputs  : 0
 */
uint32_t dir_close(uint32_t fd){
    return 0;       //"probably does nothing, return 0"
}

/* dir_read - reads files filename by filename, inluding "."
 * Inputs   : fd - file descriptor
 *          : buf - buffer we will be reading into
 *          : nbytes - number of bytes to read
 * Outputs  : >= 0 on success, -1 on failure
 */
uint32_t dir_read(uint32_t fd, void* buf, uint32_t nbytes){
    // use of global variable to keep track of which file we are on (which index)
    global_file_index = fd;
    /* use read_dentry_by_index */
    read_dentry_by_index(fd, buf);
    return 0;
}

/* dir_write - should do nothing
 * Inputs   : fd - file descriptor
 *          : buf - buffer we will be writing into
 *          : nbytes
 * Outputs  : returns -1
 */
uint32_t dir_write(uint32_t fd, void* buf, uint32_t nbytes){
    return -1;      //do nothing, return -1
}
