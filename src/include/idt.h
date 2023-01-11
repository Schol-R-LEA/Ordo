#ifndef IDT_H
#define IDT_H

#include "gdt.h"

#define IDT_SIZE 256

#define EXCEPTIONS_SIZE 32


enum IDT_gate_type: uint8_t
{
    TASK_GATE = 0x05,
    INT_GATE_16 = 0x06,
    TRAP_GATE_16 = 0x07,
    INT_GATE_32 = 0x0E,
    TRAP_GATE_32 = 0x0F
};

struct IDT_R
{
    uint16_t size;
    uint32_t base;
} __attribute__((packed));


struct Interrupt_Descriptor_32
{
    uint16_t offset_low;
    enum GDT_entry segment_selector;
    uint8_t reserved;
    struct
    {
        uint8_t gate_type:4;
        uint8_t zero:1;
        uint8_t dpl:2;
        uint8_t p:1;
    } attributes;
    uint16_t offset_high;
} __attribute__((packed));


struct Interrupt_Frame
{
    uint16_t ip;
    uint16_t cs;
    uint16_t flags;
    uint16_t sp;
    uint16_t ss;
} __attribute__((packed));


static inline void enable_interrupts()
{
    __asm__("sti");
}

static inline void disable_interrupts()
{
    __asm__("cli");
}

static inline void load_IDT(struct IDT_R* idtr)
{
    __asm__ volatile("lidt %0"
            :   /* no output argument */
            :"m"(idtr));
}


__attribute__((interrupt)) void default_interrupt_handler(struct Interrupt_Frame* frame);


void init_default_interrupts();


#endif