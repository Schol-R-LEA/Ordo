#include <stdint.h>
#include "mem_map.h"
#include "terminal.h"

#define MMAP_SPACER "   | "

const char mmap_types[][17] =
{"                ",
 "Free Memory     ",
 "Reserved Memory ",
 "ACPI Reclaimable",
 "ACPI NVS        ",
 "Bad Memory      "
};


void print_mmap(uint32_t count, struct memory_map_entry table[])
{
    kprints("\n\nMemory map (", WHITE, BLACK);
    kprintu(count, 16, WHITE, BLACK);
    kprints(" entries):\n", WHITE, BLACK);

    kprints("Base Address       | Length             | Type                    | Ext.\n", WHITE, BLACK);
    kprints("----------------------------------------------------------------------------\n", WHITE, BLACK);

    struct memory_map_entry* entry = table;
    for (uint8_t i = 0; i < count; i++, entry++)
    {
        kprintlx(entry->base, GRAY, BLACK);
        kprints(MMAP_SPACER, WHITE, BLACK);
        kprintlx(entry->length, GRAY, BLACK);
        kprints(MMAP_SPACER, WHITE, BLACK);
        kprints(mmap_types[entry->type], GRAY, BLACK);
        kprints(" (", GRAY, BLACK);
        kprintu(entry->type, 10, GRAY, BLACK);
        kprints(") ", GRAY, BLACK);
        kprints(MMAP_SPACER, WHITE, BLACK);
        kprintu(entry->ext, 10, GRAY, BLACK);
        kprintc('\n', GRAY, BLACK);
    }
}