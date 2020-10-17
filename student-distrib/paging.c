/* paging.S - set up page directory, page table, and pages */

#include "paging.h"

/* local functions */


void init_paging() {
    /* PAGE TABLE INITIALIZATION - we need to initialize this first because we need this mapping for PD */
    int i;
    for (i = 0; i < NUM_ENTRIES; i++) {
        page_table[i].P = 0;            // mark as absent
        page_table[i].R = 1;            // everything should be R/W
        page_table[i].U = 1;            // page should be able to be accessed by all
        page_table[i].W = 0;            // write-back is enabled
        page_table[i].C = 0;            // page will be cached
        page_table[i].A = 0;            // none of these accessed yet
        page_table[i].D = 0;            // hasn't been written to yet

        page_table[i].offset31_12 = 0;  // set to NULL initially
    }


    /* PAGE DIRECTORY INITIALIZATION */
    /* bit 31 in CR0 needs to be set -> PG(page enable), paging is optional so must be enabled for use */
    enable_paging();
    /* bit 4 in CR4 needs to be set -> PSE (page size extension) */
    enable_PSE();

    /* initialize same stuff first (change for vid memory and kernel later) */
    int i;
    for (i = 0; i < NUM_ENTRIES; i++) {
        page_directory[i].P = 0;        // mark as absent
        page_directory[i].R = 1;        // everything should be R/W
        page_directory[i].U = 1;        // page should be able to be accessed by all
        page_directory[i].W = 0;        // write-back is enabled
        page_directory[i].D = 0;        // page will be cached
        page_directory[i].A = 0;        // none of these have been accessed yet
        page_directory[i].S = 1;        // pages are 4 MiB in size (first PDE will eventually be set to 4kB)

        page_directory[i].offset31_12 = 0;  // set to NULL initially since these point to nowhere
    }

    /* first PDE should be for video memory */
    page_directory[0].P = 1;        // mark as present
    page_directory[0].S = 0;        // pages are 4 kB ONLY for this first PDE (due to video memory)
    

    // set right bits for first PDE
    page_directory[0].S = 0;
    // right address of Page table into this PDE
    // create pdt struct and set address in this PDE


    
    
    /* second PDE should be for kernel (at 4MB - 0x400000 for physical and virtual) */

    /* rest of these (indices 2 to 1023) is for user code */
    //int i;
    for (i = 2; i < NUM_ENTRIES; i++) {
        page_directory[i]
    }
}

void flush_tlb() {
    //asm volatile 
    //movl	%cr3,%eax
	//movl	%eax,%cr3
}

/* helper function that maps virtual memory to physical memory.
 * 
 * 
 */

