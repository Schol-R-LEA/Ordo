#include <stdint.h>
#include "gdt.h"
#include "tss.h"
#include "terminal.h"

void print_gdt_entry(union GDT_Entry *entry)
{
    uint32_t limit = entry->fields.limit_low | (entry->fields.limit_and_flags.limit_high << 16);
    uint32_t base = entry->fields.base_low | (entry->fields.base_mid << 16) | (entry->fields.base_high << 24);

    kprintf("limit: %p, base: %p\n",  limit, base);
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
    set_gdt_entry(++entry, 0x0fffff, 0, true, true, RING_0);
    // system data descriptor
    set_gdt_entry(++entry, 0x0fffff, 0, false, true, RING_0);

    // system TSS descriptor
    kprintf("Default TSS location %p\n", default_tss);
    set_gdt_entry(++entry, sizeof(struct TSS), (uint32_t) default_tss, false, true, RING_0);
    entry->fields.access.non_sys = false;

    // user code descriptor
    set_gdt_entry(++entry, 0x08ffff, 0, true, true, RING_3);
    // user data descriptor
    set_gdt_entry(++entry, 0x08ffff, 0, false, true, RING_3);


    for (uint32_t i = 0; i < 6; i++)
    {
        kprintf("%d: ", i);
        print_gdt_entry(&gdt[i]);
    }

    // set the GDT register
    set_gdt(&gdt_r);
    // reload_segments();
}