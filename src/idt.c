#include <stdint.h>
#include "idt.h"
#include "kernel.h"
#include "gdt.h"
#include "terminal.h"


void idt_set_descriptor(uint8_t vector, void isr(struct Interrupt_Frame*), enum PRIVILEGE_LEVEL dpl, enum IDT_gate_type gate_type)
{
    struct Interrupt_Descriptor_32* descriptor = &((struct Interrupt_Descriptor_32*) idt_base)[vector];

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
    panic("\nUnhandled exception.");
}


__attribute__((interrupt)) void div_zero_exception_handler(struct Interrupt_Frame* frame)
{
    disable_interrupts();
    panic("\nException 0x00: ");
}

__attribute__((interrupt)) void double_fault_exception_handler(struct Interrupt_Frame* frame)
{
    disable_interrupts();
    panic("\nException 0x08: Double Fault");
}


__attribute__((interrupt)) void page_fault_exception_handler(struct Interrupt_Frame* frame)
{
    disable_interrupts();
    panic("\nException 0x0E: Page Fault");
}


__attribute__((interrupt)) void default_interrupt_handler(struct Interrupt_Frame* frame)
{
    panic("\nUnhandled interrupt.");
}


void init_default_interrupts()
{
    // initialize the values to put into the IDT register
    __attribute__((aligned(0x10))) static struct IDT_R idt_r;

    idt_r.base = (uintptr_t) &idt_base;
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
    // idt_set_descriptor(0x0F, _handler, RING_0, INT_GATE_32);

    load_IDT(&idt_r);
}
