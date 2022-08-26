#include <stdint.h>
#include "mem_map.h"
#include "terminal.h"

#define MMAP_SPACER "     | "

char mmap_types[][16] =
{"                ",
 "Free Memory     ",
 "Reserved Memory ",
 "ACPI Reclaimable",
 "ACPI NVS        ",
 "Bad Memory      "
};


void print_mmap(uint32_t count, struct memory_map_entry* table)
{
    kprints("\n\nMemory map (", WHITE, BLACK, 0);
    kprintu(count, 16, WHITE, BLACK, 0);
    kprints(" entries):\n", WHITE, BLACK, 0);

    kprints("Base Address       | Length             | Type                  | Ext.\n", WHITE, BLACK, 0);
    kprints("----------------------------------------------------------------------------\n", WHITE, BLACK, 0);

    struct memory_map_entry* entry = table;
    for (uint8_t i = 0; i < count; i++, entry++)
    {
        kprintlx(entry->base, GRAY, BLACK, 0);
        kprints(MMAP_SPACER, WHITE, BLACK, 0);
        kprintlx(entry->length, GRAY, BLACK, 0);
        kprints(MMAP_SPACER, WHITE, BLACK, 0);
        kprints(mmap_types[entry->type], GRAY, BLACK, 0);
        kprints(" (", GRAY, BLACK, 0);
        kprintu(entry->type, 10, GRAY, BLACK, 0);
        kprints(") ", GRAY, BLACK, 0);
        kprints(MMAP_SPACER, WHITE, BLACK, 0);
        kprintu(entry->ext, 10, GRAY, BLACK, 0);
    }
}