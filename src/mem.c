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
    struct Free_List_Entry *start;
    struct Free_List_Entry *top;
    size_t size;
} heap_memory;



size_t heap_size;


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

uint8_t *get_mem_start(uint32_t count, struct boot_memory_map_entry table[])
{
    for (size_t i = count; i > 0; i--)
    {
        if (table[i].type == 1)
        {
            return (uint8_t *) (size_t) table[i].base;
        }
    }
    return nullptr;
}


uint8_t *get_mem_top(uint32_t count, struct boot_memory_map_entry table[])
{

    static uint8_t *mem_top = 0;

    if ((size_t) mem_top > 0)
    {
        return mem_top;
    }

    for (size_t i = count; i > 0; i--)
    {
        if (table[i].type == 1)
        {
            mem_top = (uint8_t *) (size_t) table[i+1].base - 1;
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



void init_heap(struct Free_List_Entry *heap_start, uint8_t* mem_top)
{
    spinlock_init(&heap_memory.lock);
    heap_memory.start = (struct Free_List_Entry *) page_round_up(heap_start);
    heap_memory.top = (struct Free_List_Entry *) page_round_down (mem_top - (PAGE_SIZE + (23 * MBYTE)));
    heap_memory.size = (size_t) heap_memory.top - (size_t) heap_memory.start;
    heap_memory.free_list = (struct Free_List_Entry *) page_round_up(heap_memory.start);

    kprintf("heap start: %p, heap end: %p, heap size: %d Bytes\n", heap_memory.start, heap_memory.top, heap_memory.size);
    kprintf("expected # of pages: %d \n", heap_memory.size / PAGE_SIZE);

    kprintf("Page ");

    for (struct Free_List_Entry *block = heap_memory.free_list; block <= heap_memory.top; block++)
    {
        uint16_t x = get_x();
        uint16_t y = get_y();
        kprintf("%u", (block - heap_memory.start));
        gotoxy(x, y);
        kfree(block);
    }

    kprintf("\n");
}

void kfree(void* start)
{
    spinlock_acquire(&heap_memory.lock);

    if (!(is_page_aligned(start)))
    {
        kprintf("\n%p: ", start);
        panic("Unaligned page address");
    }
    if ((struct Free_List_Entry *) start < heap_memory.start)
    {
        kprintf("\n%p < %p: ", start, heap_memory.start);
        panic("Page address too low");
    }
    if ((struct Free_List_Entry *) start > heap_memory.top)
    {
        kprintf("\n%p > %p: ", start, heap_memory.top);
        panic("Page address too high");
    }
    else
    {
        memset(start, 0, sizeof(struct Free_List_Entry));
        struct Free_List_Entry *block = (struct Free_List_Entry *) start;
        block->page_header.next = heap_memory.free_list;
        heap_memory.free_list = block;
    }

    spinlock_release(&heap_memory.lock);
}


/* allocate one page of memory */

void* kmalloc(size_t size)
{
    struct Free_List_Entry *block = heap_memory.free_list;

    spinlock_acquire(&heap_memory.lock);
    if (block != nullptr)
    {
        heap_memory.free_list = block->page_header.next;
    }
    spinlock_release(&heap_memory.lock);


    if (block != nullptr)
    {
        memset(block, '@', sizeof(struct Free_List_Entry));
    }

    return (void *) block;
}