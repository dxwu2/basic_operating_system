/* filesystems.h - Definitions for various structures and variables used in our filesystem */

#include "types.h"

/* macros */
#define DENTRY_B_RES        24      // 24 bytes reserved after first three elements of dir. entries
#define BOOTBLOCK_B_RES     52      // 52 bytes reserved after first three elements of boot block
#define FILENAME_LEN        32
#define NUM_DATA_BLOCKS     1023    // 1023 because each block is 4kB
                                    // each data block index is stored in 4B
                                    // therefore, we have room for 4kB/4B => 1024
                                    // but subtract 1 because length takes up first entry
#define NUM_DIRENTRIES      63      // boot block is 4kB total (each block is 4kB), 4 kB = 2^12 B
                                    // each dir entry is 64 B
                                    // therefore, we have room for 2^12/2^6 => 2^6 = 64 files
                                    // however, first 64B is reserved for some data (see struct below),
                                    // so we only have room for 63 dir entries
                                    // (first dir entry is for itself, ".", so technically only 62 dir entries)


/* data block struct */
typedef struct datablock {
    uint32_t data[NUM_DATA_BLOCKS];
} datablock_t;

/* directory entry struct (used in boot block struct) */
typedef struct dentry {
    uint8_t filename[FILENAME_LEN];
    uint32_t filetype;
    uint32_t inode_num;
    uint8_t reserved[DENTRY_B_RES];
} dentry_t;

/* boot block struct */
typedef struct boot_block {
    uint32_t dir_entry_count;
    uint32_t inode_count;       // N
    uint32_t data_block_count;  // D
    uint8_t data_reserved[BOOTBLOCK_B_RES];
    dentry_t direntries[NUM_DIRENTRIES];
} boot_block_t;

/* index node struct */
typedef struct inode {
    uint32_t length;    // length in bytes
    uint32_t data_block_num[NUM_DATA_BLOCKS];      

} inode_t;

/* the filesystem, needed?? */

/* Helper functions - utilized by local functions below and system calls */
uint32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
uint32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
uint32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

/* local functions - function params based on declarations in ece391syscall.h */
uint32_t init_file_system(void);
uint32_t file_open(const uint8_t* filename);
uint32_t file_close(uint32_t fd);
uint32_t file_read(uint32_t fd, void* buf, uint32_t nbytes);
uint32_t file_write(uint32_t fd, void* buf, uint32_t nbytes);
uint32_t dir_open(const uint8_t* filename);
uint32_t dir_close(uint32_t fd);
uint32_t dir_read(uint32_t fd, void* buf, uint32_t nbytes);
uint32_t dir_write(uint32_t fd, void* buf, uint32_t nbytes);
