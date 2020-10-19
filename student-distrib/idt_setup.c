#include "idt_setup.h"

/* void idt_setup()
 * Sets up interrupt descriptor table for inital boot
 * Inputs: None
 * Outputs: None
 * Side Effects: Fills IDT entries in table
 */
void idt_setup() {
    unsigned i;
    for(i = 0; i < NUM_VEC; i++) {  //offset_15_00 and offset_31_16 missing bc set by SET_IDT_ENTRY
        idt[i].seg_selector = KERNEL_CS;    //set all to ints to be run via kernel code seg
        idt[i].reserved4 = 0;               //reserved fields specify gate type (reserved4 is 3-bit field set to 0 for interrupts)
        idt[i].reserved3 = 0;               //01110b = interrupt gate, 01111b = trap gate
        idt[i].reserved2 = 1;
        idt[i].reserved1 = 1;
        idt[i].size      = 1;               //=1 for 32-bit gate size
        idt[i].reserved0 = 0;
        idt[i].dpl       = (i == 0x80) ? 3 : 1;   //if executing system call (vector 0x80) we must be in user privilege (3)
        idt[i].present   = 1;                //all idt desc slots are empty before SET_IDT_ENTRY called
    }

    //str for SET_IDT_ENTRY is idt_desc_t struct
    SET_IDT_ENTRY(idt[0x00], divide_err_excep);
    SET_IDT_ENTRY(idt[0x01], debug_excep);
    SET_IDT_ENTRY(idt[0x02], nmi_excep);
    SET_IDT_ENTRY(idt[0x03], breakpoint_excep);
    SET_IDT_ENTRY(idt[0x04], overflow_excep);
    SET_IDT_ENTRY(idt[0x05], bound_range_excep);
    SET_IDT_ENTRY(idt[0x06], invalid_opcode_excep);
    SET_IDT_ENTRY(idt[0x07], device_not_avail_excep);
    SET_IDT_ENTRY(idt[0x08], double_fault_excep);
    SET_IDT_ENTRY(idt[0x09], coproc_seg_overrun_excep);
    SET_IDT_ENTRY(idt[0x0A], invalid_tss_excep);
    SET_IDT_ENTRY(idt[0x0B], seg_not_present_excep);
    SET_IDT_ENTRY(idt[0x0C], stack_seg_fault_excep);
    SET_IDT_ENTRY(idt[0x0D], general_protection_excep);
    SET_IDT_ENTRY(idt[0x0E], page_fault_excep);
    //0x0F entry intel-reserved
    SET_IDT_ENTRY(idt[0x10], fpu_float_err_excep);
    SET_IDT_ENTRY(idt[0x11], align_check_excep);
    SET_IDT_ENTRY(idt[0x12], machine_check_excep);
    SET_IDT_ENTRY(idt[0x13], simd_float_err_excep);

    //We still need entries for system calls and devices
    
    // keyboard entry
    SET_IDT_ENTRY(idt[0x21], KEYBOARD_INTERRUPT);

    SET_IDT_ENTRY(idt[0x80], system_call);
}

