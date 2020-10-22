/* paging.S - set up page directory, page table, and pages */

#include "paging.h"


void init_paging(void) {

    /* bit 31 in CR0 needs to be set -> PG(page enable), paging is optional so must be enabled for use */
    /* also reloads CR3 */
    //enable_paging();
    /* TLB needs to be flushed when CR3 reloaded */
    //flush_tlb();
    /* bit 4 in CR4 needs to be set -> PSE (page size extension) */
    //enable_PSE();


    /* PAGE DIRECTORY INITIALIZATION */
    /* initialize same stuff first (change for vid memory and kernel later) */
    int i;
    for (i = 0; i < NUM_ENTRIES; i++) {
        page_directory[i].P = 0;        // mark as absent
        page_directory[i].R = 1;        // everything should be R/W
        page_directory[i].U = 1;        // page should be able to be accessed by all
        page_directory[i].W = 0;        // write-back is enabled
        page_directory[i].D = 1;        // check descriptors file: should be 1 for kernel and program pages
        page_directory[i].A = 0;        // none of these have been accessed yet
        page_directory[i].S = 1;        // pages are 4 MiB in size (first PDE will eventually be set to 4kB)

        page_directory[i].offset31_12 = 0;  // set to NULL initially since these point to nowhere
    }

    /* PAGE TABLE INITIALIZATION - we need to initialize this first because we need this mapping for PD */
    for (i = 0; i < NUM_ENTRIES; i++) {
        // check descriptors file for reason behind why bits are set this way
        page_table[i].P = 0;            // mark as absent
        page_table[i].R = 1;            // everything should be R/W
        page_table[i].U = 1;            // page should be able to be accessed by all
        page_table[i].W = 0;            // write-back is enabled
        page_table[i].C = 1;            // page-cached disabled should be set to 1??
        page_table[i].A = 0;            // none of these accessed yet
        page_table[i].D = 0;            // hasn't been written to yet

        page_table[i].offset31_12 = 0;  // set to NULL initially
    }
    /* map video memory to physical address - vid mem is at 0xB8000 and must only be ONE 4 kB page */
    // int test = 184;     //Video memory is at address 184 
    // // int test2 = VIDMEM_ADDRESS >> ADDRESS_SHIFT_KB;
    // page_table[test].P = 1;
    // page_table[test].U = 0; // vid mem should be set to supervisor only since it is kernel mapping
    // page_table[test].C = 0; // vid mem contains memory mapped I/O and shouldn't be cached

    page_table[VIDMEM_ADDRESS >> ADDRESS_SHIFT_KB].P = 1;
    page_table[VIDMEM_ADDRESS >> ADDRESS_SHIFT_KB].U = 0; // vid mem should be set to supervisor only since it is kernel mapping
    page_table[VIDMEM_ADDRESS >> ADDRESS_SHIFT_KB].C = 0; // vid mem contains memory mapped I/O and shouldn't be cached
    page_table[VIDMEM_ADDRESS >> ADDRESS_SHIFT_KB].offset31_12 = 0xB8;

    // flush_tlb();

    /* first PDE should be for video memory */
    page_directory[0].P = 1;        // mark as present
    //page_directory[0].D = 0;        // these pages contain memory mapped I/O and should not be cached
    page_directory[0].S = 0;        // pages are 4 kB ONLY for this first PDE (due to video memory)
    page_directory[0].offset31_12 = ((uint32_t) page_table >> ADDRESS_SHIFT_KB) & 0xFFF;    // set bits 31-12 to address of page table (right shifted)

    
    /* second PDE should be for kernel (at 4MB - 0x400000 for physical and virtual) */
    // page_directory[1].offset31_12 = KERNEL_ADDRESS >> ADDRESS_SHIFT_KB; 
    page_directory[1].offset31_12 = 1024; 
    page_directory[1].P = 1;        // mark as present
    page_directory[1].U = 0;        // kernel stuff should be supervisor only

    flush_tlb();

    asm volatile(
        // load page dir address into CR3 
        "movl %0, %%eax;"
        "movl %%eax, %%cr3;"

        // allow mixed page sizes: set PSE, bit 4 in CR4
        "movl %%cr4, %%eax;"
        "orl $0x00000010, %%eax;"
        "movl %%eax, %%cr4;"

        // set bit-31 of CR0 to enable paging
        "movl %%cr0, %%eax;"
        "orl $0x80000001, %%eax;"
        "movl %%eax, %%cr0"

        
        :
        : "r" (page_directory) 
        : "eax", "cc"
    );

    

}


/* void enable_paging() - paging is optional, so we need enable it by setting bit-31 in CR0
 * Inputs   : none
 * Outputs  : none
 * Clobbers : memory, cc, EAX
 * side effects : this function also locates the page directory by placing its start address in CR3
 */
/*
void enable_paging(void) {
    asm volatile(
        "movl page_directory, %%eax\n"
        "movl %%eax, %%cr3\n"
        "movl %%cr0, %%eax\n"
        "orl 0x80000001, %%eax\n"
        "movl %%eax, %%cr0"
        :
        :
        : "memory", "cc", "eax"
    );
}
*/

/* void flush_tlb() - TLB needs to be flushed when CR3 reloaded
 * Inputs : none
 * Outputs: none
 * Clobbers : memory (since we performed memory reads or writes to items other than those listed in the input and output operands)
 *          : cc (modified flag registers)
 */

void flush_tlb(void) {
    asm volatile(
        "movl %%cr3, %%eax;"
        "movl %%eax, %%cr3;"
        :
        : 
        : "eax"     // clobbered
    );
    return;
}

/* void enable_PSE()
 * Inputs  : none
 * Outputs : none
 * Clobbers: EAX,cc, mem
 * side effects : enables PSE (4 MiB pages) by setting bit 4 in CR4
 */
/*
void enable_PSE(void) {
    asm volatile(
        "movl %%cr4, %%eax\n"
        "orl 0x00000010, %%eax\n"
        "movl %%eax, %%cr4"
        :
        :
        : "memory", "cc", "eax"
    );
}
*/
