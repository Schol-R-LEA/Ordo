#ifndef GDT_H
#define GDT_H

#include <stdbool.h>
#include "kernel.h"


#define MAX_GDT_ENTRIES 0x1000
#define GDT_SIZE ((sizeof(union GDT_Entry) * MAX_GDT_ENTRIES) - 1)


enum GDT_selector: uint16_t
{
    null_selector,
    system_code_selector = (1 << 3) + RING_0,
    system_data_selector = (2 << 3) + RING_0,
    system_tss_selector  = (3 << 3) + RING_0,
    user0_code_selector  = (4 << 3) + RING_3,
    user0_data_selector  = (5 << 3) + RING_3
};


struct GDT_R
{
    uint16_t limit;
    union GDT_Entry* base;
} __attribute__ ((packed));


struct gdt_entry_fields
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    struct
    {
        bool accessed:1;
        bool read_write:1;
        bool direction_conforming:1;
        bool executable:1;
        bool non_sys:1;
        enum PRIVILEGE_LEVEL dpl:2;
        bool present:1;
    }  __attribute__((packed)) access;
    struct
    {
        uint8_t limit_high:4;
        bool reserved:1;
        bool long_mode:1;
        bool bits_32:1;
        bool granularity:1;
    }  __attribute__((packed)) limit_and_flags;
    uint8_t base_high;
} __attribute__((packed));


union GDT_Entry
{
    uint64_t raw_entry;
    struct gdt_entry_fields fields;
} __attribute__((packed));


extern union GDT_Entry gdt[];

extern void reload_segments();

void reset_gdt();


#endif