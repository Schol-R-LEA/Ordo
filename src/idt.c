#include <stdint.h>
#include "idt.h"
#include "kernel.h"
#include "gdt.h"
#include "terminal.h"

__attribute__((aligned(0x10))) static struct IDT_R idt_r;
__attribute__((aligned(0x10))) static struct Interrupt_Descriptor_32 idt[IDT_SIZE];


void idt_set_descriptor(uint8_t vector, void isr(struct Interrupt_Frame*), uint8_t dpl, enum IDT_gate_type gate_type) {
    struct Interrupt_Descriptor_32* descriptor = &idt[vector];

    descriptor->offset_low       = (uint32_t)isr & 0xFFFF;
    descriptor->segment_selector = system_code_entry;
    descriptor->attributes.p     = 1;
    descriptor->attributes.dpl   = dpl;
    descriptor->attributes.zero  = 0;
    descriptor->attributes.gate_type  = gate_type;
    descriptor->offset_high      = (uint32_t)isr >> 16;
    descriptor->reserved         = 0;
}


__attribute__((interrupt)) void default_exception_handler(struct Interrupt_Frame* frame)
{
    disable_interrupts();
    kprints("Unhandled exception.", WHITE, BLACK, 0);
    __asm__("hlt");
}



__attribute__((interrupt)) void default_interrupt_handler(struct Interrupt_Frame* frame)
{
    disable_interrupts();
    kprints("Unhandled interrupt.", WHITE, BLACK, 0);

    __asm__("hlt");
}


void init_default_interrupts()
{
    idt_r.base = (uint32_t) &idt;
    idt_r.size = (uint16_t) (sizeof(struct Interrupt_Descriptor_32) * IDT_SIZE) - 1;

    for (uint8_t vector = 0; vector < (IDT_SIZE - 1); vector++)
    {
        idt_set_descriptor(vector,
                           (vector < EXCEPTIONS_SIZE
                            ? default_exception_handler : default_interrupt_handler),
                           RING_0,
                           (vector < EXCEPTIONS_SIZE
                            ? TRAP_GATE_32 : INT_GATE_32));
    }

    load_IDT(&idt_r);
}
