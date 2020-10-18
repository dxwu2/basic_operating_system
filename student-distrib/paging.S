# paging.S - includes functions that are to be written in assembly with regards to paging

#define ASM     1

.text

.globl enable_paging
.globl enable_PSE
.globl flush_tlb

/* void enable_paging() - paging is optional, so we need enable it by setting bit-31 in CR0
 * Inputs   : none
 * Outputs  : none
 * side effects : this function also locates the page directory by placing its start address in CR3
 */
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

/* void flush_tlb()
 * Inputs : none
 * Outputs: none
 * side effects : TLB needs to be flushed whenever cr3 is reloaded (program switches)
 */
flush_tlb:
    movl %cr3, %eax
    movl %eax, %cr3
    ret