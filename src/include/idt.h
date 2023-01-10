#ifndef IDT_H
#define IDT_H

enum IDT_gate_type
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
    uint8_t offset;
};

struct IDT_Gate_Descriptor
{
    uint16_t offset_low;
    uint16_t segment_selector;
    uint8_t reserved;
    struct
    {
        uint8_t p:1;
        uint8_t dpl:2;
        uint8_t zero:1;
        uint8_t gate_type:4;
    } tfields;
    uint16_t offset_high;
} __attribute__((packed));

#endif