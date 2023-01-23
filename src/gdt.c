#include <stdint.h>
#include "gdt.h"
#include "tss.h"

union GDT_Entry *set_gdt_entry(union GDT_Entry * entry, uint32_t limit, uint32_t base, bool exec, bool rw, enum PRIVILEGE_LEVEL dpl)
{
    entry->fields.limit_low = limit & 0xffff;
    entry->fields.base_low = base & 0xffff;
    entry->fields.base_mid = (base >> 16) & 0xffff;
    entry->fields.access.accessed = 0;
    entry->fields.access.read_write = rw;
    entry->fields.access.direction_conforming = false;
    entry->fields.access.executable = exec;
    entry->fields.access.non_sys = true;
    entry->fields.access.dpl = dpl;
    entry->fields.access.present = true;
    entry->fields.limit_and_flags.limit_high = (limit >> 16) & 0xffff;
    entry->fields.limit_and_flags.reserved = false;
    entry->fields.limit_and_flags.long_mode = false;
    entry->fields.limit_and_flags.bits_32 = true;
    entry->fields.limit_and_flags.granularity = false;

    entry->fields.base_high = base >> 24;

    return entry;
}


void reset_gdt()
{
    struct GDT_R gdt_r = { sizeof(union GDT_Entry) * MAX_GDT_ENTRIES, gdt };

    union GDT_Entry *entry = gdt;
    // set the null GDT entry
    entry->raw_entry = 0;
    // system code descriptor
    set_gdt_entry(++entry, 0x0fffff, 0, true, true, RING_0);
    // system data descriptor
    set_gdt_entry(++entry, 0x0fffff, 0, false, true, RING_0);
    // system TSS descriptor
    set_gdt_entry(entry, (uint32_t) default_tss, sizeof(struct TSS), false, true, RING_0);
    (entry++)->fields.access.non_sys = false;
    // user code descriptor
    set_gdt_entry(++entry, 0x08ffff, 0, true, true, RING_3);
    // user data descriptor
    set_gdt_entry(++entry, 0x08ffff, 0, false, true, RING_3);

    // set the GDT register
    __asm__ __volatile__ (
        "lgdt %0"
        :
        : "m" (gdt_r));
}