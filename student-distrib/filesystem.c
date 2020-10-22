/* filesystem.c - sets up filesystem, includes some other functions regarding fs */

#include "filesystem.h"


/* int32_t file_write(int32_t fd, void* buf, int32_t nbytes) - should do nothing
 * Inputs   : fd, buf, nbytes
 * Outputs  : -1 since our file system is read-only (as stated in Discussion week 9)
 */
int32_t file_write(int32_t fd, void* buf, int32_t nbytes){
    return -1;      //do nothing, return -1
}

int32_t dir_write(int32_t fd, void* buf, int32_t nbytes){
    return -1;      //do nothing, return -1
}

int32_t dir_close(int32_t fd){
    return 0;       //"probably does nothing, return 0."
}