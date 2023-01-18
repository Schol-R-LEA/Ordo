#include <stdint.h>
#include "idt.h"
#include "kernel.h"
#include "gdt.h"
#include "terminal.h"


__attribute__((aligned(0x10))) static struct Interrupt_Descriptor_32 idt[IDT_SIZE];


struct Exception_Description_Entry
{
    char *isr_name, *isr_desc;
};

// lookup table for exception descriptions
// With thanks to Nicholas Kelly for the original at
// https://github.com/krisvers/kros/blob/master/kernel/arch/x64/isr.c

/*
static const struct Exception_Description_Entry Exception_Descriptions = {
    {"Div0","Divide by zero"},
    {"Debug", "Debug"},
    {"NMI", "Non-maskable INT"},
    {"Breakpoint", "Breakpoint"},
    {"Overflow", "Overflow"},
    {"BoundedRange", "Bound Range Exceeded"},
    {"InvalidOpcode", "Invalid Opcode"},
    {"NoDevice", "Device Not Available"},
    {"DoubleFault", "Double Fault"},
    {"CoprocessorSegmentOverrun", "Coprocessor Segment Overrun"},
    {"InvalidTSS", "Invalid TSS"},
    {"MissingSegment", "Segment Not Present"},
    {"StackFault", "Stack-Segment Fault"},
    {"GPF", "General Protection Fault"},
    {"PageFault", "Page Fault"},
    {"UnusedException_0x0F", ""},
    {"FPError", "x87 Floating-Point Exception"},
    {"Alignment", "Alignment Check"},
    {"MachineCheck", "Machine Check"},
    {"SIMDError", "SIMD Floating-Point Exception"},
    {"Virtualization", "Virtualization Exception"},
    {"CPE", "Control Protection Exception"},
    {"UnusedException_0x17", "UnusedException_0x17"},
    {"UnusedException_0x18", "UnusedException_0x18"},
    {"UnusedException_0x19", "UnusedException_0x19"},
    {"UnusedException_0x1A", "UnusedException_0x1A"},
    {"UnusedException_0x1B", "UnusedException_0x1B"},
    {"UnusedException_0x1C", "UnusedException_0x1C"},
    {"HVInjection", "Hypervisor Injection Exception"},
    {"VMMComm", "VMM Communication Exception"},
    {"Security", "Security Exception"}
};

#define GENERATE_ISR(number, name, msg) __attribute__((interrupt)) void default_  name ## _exception_handler(struct Interrupt_Frame* frame) \
{ \
    disable_interrupts(); \
    set_fg(WHITE); \
    kprintf("\nException %u: ", number, msg); \
    panic(); \
} */

void idt_set_descriptor(uint8_t vector, void isr(struct Interrupt_Frame*), enum PRIVILEGE_LEVEL dpl, enum IDT_gate_type gate_type)
{
    struct Interrupt_Descriptor_32* descriptor = &idt[vector];

    descriptor->offset_low       = (uint16_t) ((uint32_t) isr & 0xFFFF);
    descriptor->segment_selector = system_code_selector;
    descriptor->attributes.p     = 1;
    descriptor->attributes.dpl   = dpl;
    descriptor->attributes.zero  = 0;
    descriptor->attributes.gate_type = gate_type;
    descriptor->offset_high      = (uint16_t) ((uint32_t) isr >> 16);
    descriptor->reserved         = 0;
}

__attribute__((interrupt)) void default_exception_handler(struct Interrupt_Frame* frame)
{
    disable_interrupts();
    kprints("\nUnhandled exception.", WHITE, BLACK);
    panic();
}


__attribute__((interrupt)) void div_zero_exception_handler(struct Interrupt_Frame* frame)
{
    disable_interrupts();
    kprints("\nException 0x00: ", WHITE, BLACK);
    panic();
}

__attribute__((interrupt)) void double_fault_exception_handler(struct Interrupt_Frame* frame)
{
    disable_interrupts();
    kprints("\nException 0x08: Double Fault", WHITE, BLACK);
    panic();
}


__attribute__((interrupt)) void page_fault_exception_handler(struct Interrupt_Frame* frame)
{
    disable_interrupts();
    kprints("\nException 0x0E: Page Fault", WHITE, BLACK);
    panic();
}



__attribute__((interrupt)) void default_interrupt_handler(struct Interrupt_Frame* frame)
{
    kprints("\nUnhandled interrupt.", WHITE, BLACK);
    panic();
}


void init_default_interrupts()
{
    // initialize the values to put into the IDT register
    __attribute__((aligned(0x10))) static struct IDT_R idt_r;

    idt_r.base = (uintptr_t) &idt[0];
    idt_r.size = (uint16_t) sizeof(struct Interrupt_Descriptor_32) * IDT_SIZE - 1;

    // As a precaution, populate the IDT with default ISRs.
    // This is done in two phases, first populating the exception handlers,
    // then populating the interrupt handlers.
    for (uint8_t vector = 0; vector < (EXCEPTIONS_SIZE - 1); vector++)
    {
        idt_set_descriptor(vector, default_exception_handler, RING_0, TRAP_GATE_32);
    }

    for (uint8_t vector = EXCEPTIONS_SIZE; vector < (IDT_SIZE - 1); vector++)
    {
        idt_set_descriptor(vector, default_interrupt_handler, RING_0, INT_GATE_32);
    }


    // add exception-specific handlers
    idt_set_descriptor(0x00, div_zero_exception_handler, RING_0, TRAP_GATE_32);
    idt_set_descriptor(0x08, double_fault_exception_handler, RING_0, TRAP_GATE_32);
    idt_set_descriptor(0x0E, page_fault_exception_handler, RING_0, TRAP_GATE_32);
    // add interrupt-specific handlers

    load_IDT(&idt_r);
}
