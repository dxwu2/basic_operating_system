/* filesystem.c - sets up filesystem, includes some other functions regarding fs */

#include "filesystem.h"
#include "lib.h"


/* File system initialization
 * Only one module loaded in c, so it must be filesystem
 * Call init_file_system() from kernel.c using mod captured from multiboot info mbi
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

    else
        the_boot_block = potential_boot_block;      //officially set local boot block to fs_start
    return 0;
}

uint32_t file_open(const uint8_t* filename) {
    return 0;
}

/* Helper functions defined */
uint32_t read_dentry_by_name (const int8_t* fname, dentry_t* dentry){
    unsigned i;     //for loop below
    /* Check if filesystem initialized */
    if(!the_boot_block)
        return -1;
    /* Check inputs for validity */
    if((!fname) || (!dentry) || (strlen(fname) > FILENAME_LEN))
        return -1;
    /* scan through dentries in boot block to find fname */
    int8_t fname_len = strlen(fname);
    for(i = 0; i < NUM_DIRENTRIES; i++){
        dentry_t* potential_dentry = &(the_boot_block->direntries[i]);
        if(!strncmp(fname, potential_dentry->filename, fname_len)){
            *dentry = *potential_dentry;
            return 0;
        }
    }
    return -1;      //We could not find dentry with desired filename
}

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
    //return -1 in event of inode outside valid range or if bad data block number in inode
    /* check if filesystem not initialized or if inode number is invalid */
    if (!the_boot_block || !buf || inode > (the_boot_block->inode_count) - 1) {
        return -1;
    }

    int len = (int) length;

    /* retrieve inode */
    unsigned char* i = the_boot_block + BLOCK_SIZE * (inode + 1);     // + 1 because of the boot block
    unsigned char* next = the_boot_block + BLOCK_SIZE * (inode + 2);  // for endpoint

    /* apply offset */
    /*
    if (offset > (inode_t*)i->length) {
        return 0;
    }
    */
   if (offset > len) {
       return 0;
   }
    // if end of file will be reached
    if (i + offset + length >= next) {
        memcpy(i + offset, buf, next - (i + offset));
        return 0;
    }
    // if end of file won't be reached
    else {
        memcpy(i + offset, buf, length);
    }
    // return number of bytes read
    return length;
}


/* Local functions defined
 * Define file operation functions separate from directory operations 
 * As stated in Discussion Week 9, return -1 for any write functions 
 */
 /* uint32_t file_open(const uint8_t* filename) - initializes any temporary structures
  * Inputs  : filename
  * Outputs : returns 0
  */
 /*
uint32_t file_open(const int8_t* filename){
    return 0;
}
*/

uint32_t file_close(uint32_t fd){
    return 0;
}

uint32_t file_read(uint32_t fd, void* buf, uint32_t nbytes){
    /* use read_data */
    return 0;
}

uint32_t file_write(uint32_t fd, void* buf, uint32_t nbytes){
    return -1;      //do nothing, return -1
}


/* Separate functions for directory operations */
uint32_t dir_open(const int8_t* filename){
    /* use read_dentry_by_name */
    return 0;
}

uint32_t dir_close(uint32_t fd){
    return 0;       //"probably does nothing, return 0"
}

uint32_t dir_read(uint32_t fd, void* buf, uint32_t nbytes){
    /* use read_dentry_by_index */
    return 0;
}

uint32_t dir_write(uint32_t fd, void* buf, uint32_t nbytes){
    return -1;      //do nothing, return -1
}
