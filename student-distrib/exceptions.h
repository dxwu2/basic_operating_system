#ifndef _EXCEPTIONS_H
#define _EXCEPTIONS_H

#include "lib.h"

/* prints exception message based on vector number */
void handle_exception(unsigned vec);

 /* Temporary function before actual system calls implemented that prints system call message */
void handle_system_call();

/* array of exception message strings */
const char* exception_msgs[20];

/*exception handlers (used as pointers by idt) - just route each exception through handle_exception function for now*/
void divide_err_excep();
void debug_excep();
void nmi_excep();
void breakpoint_excep();
void overflow_excep();
void bound_range_excep();
void invalid_opcode_excep();
void device_not_avail_excep();
void double_fault_excep();
void coproc_seg_overrun_excep();
void invalid_tss_excep();
void seg_not_present_excep();
void stack_seg_fault_excep();
void general_protection_excep();
void page_fault_excep();
void fpu_float_err_excep();
void align_check_excep();
void machine_check_excep();
void simd_float_err_excep();

/* System call handler declaration */
void system_call();
#endif

//BUGS: DEFINING STUFF in header rather than just declaring

//add extern before declarations???

// had to remove exception.S assembly file (stuff compiling twice)

//INVALID MAGIC NUMBER: wasn't saving caller-saved registers before initializing idt

//Print statement exception handler did not appear - had to add clear() in exception handler
