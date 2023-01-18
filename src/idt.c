#include <stdint.h>
#include "idt.h"
#include "kernel.h"
#include "gdt.h"
#include "terminal.h"


__attribute__((aligned(0x10))) static struct Interrupt_Descriptor_32 idt[IDT_SIZE];

// lookup table for exception descriptions
// With thanks to Nicholas Kelly for the original at
// https://github.com/krisvers/kros/blob/master/kernel/arch/x64/isr.c
/* static const char * const Exception_Descriptions = {
    "Divide by zero",
    "Debug",
    "Non-maskable INT",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "",
    "",
    "",
    "",
    "",
    "",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    ""
}; */



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
    kprints("\nUnhandled exception.", WHITE, BLACK, 0);
    panic();
}


__attribute__((interrupt)) void div_zero_exception_handler(struct Interrupt_Frame* frame)
{
    disable_interrupts();
    kprints("\nException 0x00: ", WHITE, BLACK, 0);
    panic();
}

__attribute__((interrupt)) void double_fault_exception_handler(struct Interrupt_Frame* frame)
{
    disable_interrupts();
    kprints("\nException 0x08: Double Fault", WHITE, BLACK, 0);
    panic();
}


__attribute__((interrupt)) void page_fault_exception_handler(struct Interrupt_Frame* frame)
{
    disable_interrupts();
    kprints("\nException 0x0E: Page Fault", WHITE, BLACK, 0);
    panic();
}





__attribute__((interrupt)) void default_interrupt_handler(struct Interrupt_Frame* frame)
{
    kprints("\nUnhandled interrupt.", WHITE, BLACK, 0);
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
