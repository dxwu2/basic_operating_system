/* paging.h - Defines for various paging stuff: paging directory, paging table, etc.
 */

#include "types.h"
#include "x86_desc.h"   // most likely???

/* macros */
#define PAGE_DIR_SIZE 1024

/* struct for each Page Directory Entry */
typedef struct pde {
    uint32_t pd_entry;
    int idx;
    unsigned char * page_table;
} pde;

/* struct for each Page Table Entry */
typedef struct pte {
    uint32_t pt_entry;
    int idx;
    //unsigned char * page; -> 32 bit thingy is actually a PTE
} pte;

/* global variables */
pde page_directory[PAGE_DIR_SIZE];

/* functions */
void init_paging();
// maybe function to create pde?? maybe not
// maybe function to create pte??
void flush_tlb();