/* paging.S - set up page directory, page table, and pages */

#include "paging.h"


void init_paging() {
    // bit 31 in CR0 needs to be set -> PG(page enable), paging is optional so must be enabled for use
    // bit 4 in CR4 needs to be set -> PSE (page size extension)
    // bit 7 in PDE needs to be set-> PS (page size), needs for loop??
}

void flush_tlb() {
    //asm volatile 
    //movl	%cr3,%eax
	//movl	%eax,%cr3
}

