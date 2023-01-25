#include <stdint.h>
#include "gdt.h"
#include "tss.h"
#include "terminal.h"



void print_gdt_entry(union GDT_Entry *entry)
{
    uint32_t limit = entry->fields.limit_low | (entry->fields.limit_and_flags.limit_high << 16);
    uint32_t base  = entry->fields.base_low  | (entry->fields.base_mid << 16) | (entry->fields.base_high << 24);

    uint64_t access = (entry->raw_entry >> 40) & 0xff;
    uint64_t flags  = (entry->raw_entry >> 52) & 0x0f;

    kprintf("%p %p %l %l %l\n", limit, base, access, flags, entry->raw_entry);
}


void print_gdt(union GDT_Entry *gdt)
{
    kprintf("\t  limit    base     access           flags            whole\n");
    for (uint32_t i = 0; i < 6; i++)
    {
        kprintf("%d:\t", i);
        print_gdt_entry(&gdt[i]);
    } 
}


void set_gdt_entry(union GDT_Entry *entry, uint32_t limit, uint32_t base, bool exec, bool rw, enum PRIVILEGE_LEVEL dpl)
{
    entry->fields.limit_low = limit & 0xffff;        // get lower 16 bits of limit
    entry->fields.base_low = base & 0xffff;          // get lower 16 bits of base
    entry->fields.base_mid = (base >> 16) & 0xff;    // get middle 8 bits of base

    entry->fields.access.accessed = false;
    entry->fields.access.read_write = rw;
    entry->fields.access.direction_conforming = false;
    entry->fields.access.executable = exec;
    entry->fields.access.non_sys = true;
    entry->fields.access.dpl = dpl;
    entry->fields.access.present = true;

    // get highest 4 bits of limit
    entry->fields.limit_and_flags.limit_high = (limit >> 16) & 0x0f;
    entry->fields.limit_and_flags.reserved = false;
    entry->fields.limit_and_flags.long_mode = false;
    entry->fields.limit_and_flags.bits_32 = true;
    entry->fields.limit_and_flags.granularity = true; // always use page granularity

    entry->fields.base_high = base >> 24;             // get highest 8 bits of base
}


void reset_gdt()
{
    struct GDT_R gdt_r = { GDT_SIZE, gdt };

    union GDT_Entry *entry = gdt;

    // first, clear the whole table
    memset(entry, 0, GDT_SIZE);

    // set the null GDT entry = redundant, but still worth doing
    entry->raw_entry = 0;
    // system code descriptor
    set_gdt_entry(++entry, 0x000fffff, 0, true, true, RING_0);
    // system data descriptor
    set_gdt_entry(++entry, 0x000fffff, 0, false, true, RING_0);

    // system TSS descriptor
    kprintf("\nDefault TSS location %p\n", &default_tss);
    set_gdt_entry(++entry, sizeof(struct TSS), (uint32_t) &default_tss, true, false, RING_0);
    entry->fields.access.accessed = true;
    entry->fields.access.non_sys = false;
    entry->fields.limit_and_flags.bits_32 = false;
    entry->fields.limit_and_flags.granularity = false;

    // user code descriptor
    set_gdt_entry(++entry, 0x0008ffff, 0, true, true, RING_3);
    // user data descriptor
    set_gdt_entry(++entry, 0x0008ffff, 0, false, true, RING_3);


    print_gdt(gdt);


    // set the GDT register
 
    __asm__ __volatile__ (
        "        lgdt %0;"
        :
        : "m" (gdt_r));

    reload_segments();
}