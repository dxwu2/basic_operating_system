#ifndef _IDT_SETUP_H
#define _IDT_SETUP_H

#include "x86_desc.h"
#include "exceptions.h"

/* sets up and fills interrupt descriptor table entries/gates */
void idt_setup(void);

// for assembly linkage
void KEYBOARD_INTERRUPT(void);
void RTC_INTERRUPT(void);

#endif
