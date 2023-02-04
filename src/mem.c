#include <stdint.h>
#include <stddef.h>
#include "mem.h"
#include "paging.h"
#include "terminal.h"

#define MMAP_SPACER "   | "

const char boot_mmap_types[][17] =
{"                ",
 "Free Memory     ",
 "Reserved Memory ",
 "ACPI Reclaimable",
 "ACPI NVS        ",
 "Bad Memory      "
};


extern struct PMM_Entry PMM_table[PT_ENTRY_TOTAL_COUNT];


void print_boot_mmap(uint32_t count, struct boot_memory_map_entry table[])
{
    set_fg(WHITE);
    kprintf("\n\nMemory map %d entries):\n", count);

    kprintf("Base Address       | Length             | Type                    | Ext.\n");
    kprintf("--------------------------------------------------------------------------\n");

    set_fg(GRAY);
    struct boot_memory_map_entry* entry = table;
    for (uint8_t i = 0; i < count; i++, entry++)
    {
        kprintf("%l", entry->base);
        kprints(MMAP_SPACER, WHITE, BLACK);
        kprintf("%l", entry->length);
        kprints(MMAP_SPACER, WHITE, BLACK);
        kprintf("%s", boot_mmap_types[entry->type]);
        kprintf(" (%d) ", entry->type);
        kprints(MMAP_SPACER, WHITE, BLACK);
        kprintf(" %d\n", entry->ext);
    }
}


void* memset(void *ptr, char value, size_t num)
{
    uint8_t *p = (uint8_t *)ptr;

    while (num--)
    {
        *(p++) = value;
    }

    return ptr;
}


void* memcpy(void *destination, void *source, size_t num)
{
    uint8_t *dest = (uint8_t*)destination;
    uint8_t *src = (uint8_t*)source;

    while (num--)
    {
        *(dest++) = *(src++);
    }

    return destination;
}



#define LINE_SIZE 16

void dump_line(void* src, uint8_t size)
{
    uint8_t* p = (uint8_t *) src;

    for (uint8_t i= 0, *b = p; i < size; i++, b++)
    {
        if (i == (LINE_SIZE / 2))
        {
            kprintf("\t");
        }
        if (*b < 0x10)
        {
            kprintf("0");
        }
        kprintf("%x ", *b);
    }
    if (size < LINE_SIZE)
    {
        for (uint8_t i = 0; i < (LINE_SIZE - size); i++)
        {
            kprintf("   ");
        }
        if (size < (LINE_SIZE / 2))
        {
            kprintf("\t");
        }
    }

    kprintf("\t");

    for (uint8_t i = 0, *b = p; i < size; i++, b++)
    {
        if (i == LINE_SIZE / 2)
        {
            kprintc('-', GREEN, BLACK);
        }
        kprintf("%c", ((*b < 'a' || *b > 0xf0) ? '.' : *b));
    }

    kprintf("\n");
}


void memdump(void* src, uint32_t size)
{ 
    uint8_t* p = (uint8_t *) src;
    uint32_t remainder = size % LINE_SIZE;
    uint32_t lines = size / LINE_SIZE;


    for (uint32_t i = 0; i < lines; i++, p += LINE_SIZE)
    {
        dump_line(p, LINE_SIZE);
    }

    p += LINE_SIZE;
    if (remainder != 0)
    {
        dump_line(p, remainder);
    }
}


size_t get_total_mem(uint32_t count, struct boot_memory_map_entry table[])
{
    static size_t total_mem_size = 0;

    if (total_mem_size > 0)
    {
        return total_mem_size;
    }

    for (size_t i = 0; i < count; i++)
    {
        if (table[i].type == 1)
        {
            total_mem_size += table[i].length;
        }
    }

    return total_mem_size;
}



void init_physical_memory_map(uint32_t count, struct boot_memory_map_entry table[])
{
    static bool initialized = false;

    if (initialized)
    {
        return;
    }

    for(size_t i = 0; i < count; i++)
    {
        if (table[i].type != 1)
        {
            for (size_t j = table[i].base / 0x1000; j < table[i].length / 0x1000; j++)
            {
                set_pmm_entry(j);
            }
        }
        else
        {
            for (size_t j = table[i].base / 0x1000; j < table[i].length / 0x1000; j++)
            {
                clear_pmm_entry(j);
            }
        }
    }

    initialized = true;

    memdump(pmm_table, 256);
}