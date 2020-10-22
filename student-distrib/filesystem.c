/* filesystem.c - sets up filesystem, includes some other functions regarding fs */

#include "filesystem.h"

/* local static for boot block */
static boot_block_t the_boot_block;

/* File system initialization */
uint32_t init_file_system(){
    the_boot_block.dir_entry_count = 0;

}

uint32_t file_open(const uint8_t* filename) {

}

/* Helper functions defined */
uint32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry){
    /* Check inputs to ensure validity */
    if((!fname) || (!dentry)){
        return -1;
    }
    /* scan through dentries in boot block to find fname */
    /* Call read_dentry_by_index */
    return 0;
}

uint32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){
    /* Check inputs for validity */
    if((index > the_boot_block.dir_entry_count) || (!dentry)){
        return -1;
    }
    /* populate dentry parameter -> file name, file type, inode number */
    
    return 0;
}

/* read_data - reads up to "length" bytes starting from "offset" position in the file with inode number "inode"
 * Inputs   : inode - indicates which file
 *          : offset - tells us where to start reading in the file
 *          : buf - read info placed in here
 *          : length - tell us how many bytes to read
 * Outputs  : returns the number of bytes read, or 0 if end of file reached, or -1 on failure
 * Side effects : bytes read are placed in the buffer
 */
uint32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    /* check if inode number is invalid */
    if (inode > (the_boot_block.N) - 1) {
        return -1;
    }


    return 0;
}


/* Local functions defined
 * Define file operation functions separate from directory operations 
 * As stated in Discussion Week 9, return -1 for any write functions 
 */
 /* uint32_t file_open(const uint8_t* filename) - initializes any temporary structures
  * Inputs  : filename
  * Outputs : returns 0
  */
uint32_t file_open(const uint8_t* filename){
    return 0;
}

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
uint32_t dir_open(const uint8_t* filename){
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