# paging.S - includes functions that are to be written in assembly with regards to paging

#define ASM     1
#include "paging.h"

enable_paging:
    movl page_directory, %eax
    movl %eax, %cr3
    movl %cr0, %eax
    orl 0x80000001, %eax
    movl %eax, %cr0
    ret

/* void enable_PSE()
 * Inputs  : none
 * Outputs : none
 * side effects : enables PSE (4 MiB pages) by setting bit 4 in CR4
 */
enable_PSE:
    movl %cr4, %eax
    orl eax, 0x00000010, %eax
    movl %eax, %cr4
    ret
