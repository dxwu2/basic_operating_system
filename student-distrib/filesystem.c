/* filesystem.c - sets up filesystem, includes some other functions regarding fs */

#include "filesystem.h"

/* As stated in Discussion Week 9, return -1 for write functions */
int32_t file_write(int32_t fd, void* buf, int32_t nbytes){
    return -1;      //do nothing, return -1
}

int32_t dir_write(int32_t fd, void* buf, int32_t nbytes){
    return -1;      //do nothing, return -1
}

int32_t dir_close(int32_t fd){
    return 0;       //"probably does nothing, return 0."
}