#include <stdint.h>
#include <stddef.h>
#include "mem.h"
#include "paging.h"
#include "terminal.h"
#include "spinlock.h"

#define MMAP_SPACER "   | "


const char boot_mmap_types[][17] =
{"                ",
 "Free Memory     ",
 "Reserved Memory ",
 "ACPI Reclaimable",
 "ACPI NVS        ",
 "Bad Memory      "
};

struct
{
    spinlock lock;
    struct Free_List_Entry *free_list;
} kernel_memory_info;


struct Free_List_Entry *heap_start, *heap_top;

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

size_t *get_mem_start(uint32_t count, struct boot_memory_map_entry table[])
{
    for (size_t i = count; i > 0; i--)
    {
        if (table[i].type == 1)
        {
            return (size_t *) (size_t) table[i].base;
        }
    }
    return nullptr;
}


size_t *get_mem_top(uint32_t count, struct boot_memory_map_entry table[])
{

    static size_t *mem_top = 0;

    if ((size_t) mem_top > 0)
    {
        return mem_top;
    }

    for (size_t i = count; i > 0; i--)
    {
        if (table[i].type == 1)
        {
            mem_top = (size_t *) (size_t) table[i+1].base - 1;
            break;
        }
    }
    return mem_top;
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
}



size_t init_heap(size_t* mem_start, size_t* mem_top)
{
    spinlock_init(&kernel_memory_info.lock);
    heap_start = (struct Free_List_Entry *) mem_start;
    heap_top = (struct Free_List_Entry *) mem_top;
    kernel_memory_info.free_list = (struct Free_List_Entry *) page_round_up(heap_start);
    size_t page_count = 0;
    for (struct Free_List_Entry* block = kernel_memory_info.free_list;
         //block < heap_top - sizeof(struct Free_List_Entry);
         page_count < 768;
         block++)
    {
        kfree(block);
        page_count++;
    }

    return page_count;
}

void kfree(void* start)
{
    spinlock_acquire(&kernel_memory_info.lock);

    if (!(is_page_aligned(start) && addr_in_range(start, heap_start, heap_top)))
    {
        kprintf("unaligned page address: %p\n", start);
        panic("Unaligned page address");
    }
    if ((size_t) start < (size_t) heap_start)
    {
        kprintf("page address too low: %p\n", start);
        panic("Page address out of range");
    }
    if ((size_t) start > (size_t) (heap_top - sizeof(struct Free_List_Entry)))
    {
        kprintf("page address too high: %p > %p\n", start, heap_top);
        panic("Page address out of range");
    }
    else
    {
        memset(start, 0, sizeof(struct Free_List_Entry));
        //struct Free_List_Entry *block = (struct Free_List_Entry *) start;
        //block->page_header.next = kernel_memory_info.free_list;
        //kernel_memory_info.free_list = block;
    }

    spinlock_release(&kernel_memory_info.lock);
}


/* allocate one page of memory */

void* kmalloc(size_t size)
{
    struct Free_List_Entry *block = kernel_memory_info.free_list;

    spinlock_acquire(&kernel_memory_info.lock);
    if (block != nullptr)
    {
        kernel_memory_info.free_list = block->page_header.next;
    }
    spinlock_release(&kernel_memory_info.lock);


    if (block != nullptr)
    {
        memset(block, '@', sizeof(struct Free_List_Entry));
    }

    return (void *) block;
}