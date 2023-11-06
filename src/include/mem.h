#ifndef MEM_H
#define MEM_H

#include <stddef.h>
#include <stdint.h>
#include "spinlock.h"

#define KBYTE 1024
#define MBYTE (KBYTE * KBYTE)
#define GBYTE (MBYTE * KBYTE)
#define PAGE_SIZE (KBYTE * 4)
#define PAGE_TOP (PAGE_SIZE - 1)
#define PAGE_DATA_SIZE (PAGE_SIZE - sizeof(struct Page_Header*))


#define pmm_bitwidth (sizeof(pmm_table[0]) * 8)


struct boot_memory_map_entry
{
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t ext;
} __attribute__ ((packed));


enum PM_Type : uint8_t
{
    NONE, RESERVED, FIRMWARE, MMIO, SYSTEM, HEAP
};


enum VM_Type : uint8_t
{
    PAGING = 6, SYSTEM_CODE, SYSTEM_DATA, USER_CODE, USER_DATA
};


struct PMM_Entry
{
    struct PMM_Entry *previous;
    struct PMM_Entry *next;
    enum PM_Type type;
    size_t address;
    size_t span;
};


struct Page_Header
{
    struct Free_List_Entry *next;
} __attribute__((packed));


struct Free_List_Entry
{
    struct Page_Header page_header;
    uint8_t data[PAGE_SIZE - sizeof(struct Page_Header)];
} __attribute__((packed));



extern uint64_t pmm_table[];
extern uint8_t pmm_table_size;

extern uint8_t *kernel_physical_base;
extern uint8_t *kernel_physical_top;
extern struct Free_List_Entry *heap;

static inline void* page_round_up(void* value)
{
    return (void*) (((size_t) value + PAGE_TOP) & ~(PAGE_TOP));
}


static inline void* page_round_down(void* value)
{
    return (void*) ((size_t) value & ~(PAGE_TOP));
}


static inline bool is_page_aligned(void* address)
{
    return ((size_t) address % PAGE_SIZE) == 0;
}


static inline bool addr_in_range(void* address, void* low, void* high)
{
    return (address >= low) && (address < high);
}



static inline void set_pmm_entry(size_t index)
{
    size_t byte_index = index / pmm_bitwidth;
    size_t bit_index = index % pmm_bitwidth;

    pmm_table[byte_index] = pmm_table[byte_index] | (1 << bit_index);
}


static inline void clear_pmm_entry(size_t index)
{
    size_t byte_index = index / 64;
    size_t bit_index = index % 64;

    pmm_table[byte_index] = pmm_table[byte_index] & ~(1 << bit_index);
}


static inline bool get_pmm_entry(size_t index)
{
    size_t byte_index = index / 64;
    size_t bit_index = index % 64;

    return (bool) ((pmm_table[byte_index] & (1 << bit_index)) >> bit_index);
}


void print_boot_mmap(uint32_t count, struct boot_memory_map_entry table[]);
size_t get_total_mem(uint32_t count, struct boot_memory_map_entry table[]);
uint8_t *get_mem_start(uint32_t count, struct boot_memory_map_entry table[]);
uint8_t *get_mem_top(uint32_t count, struct boot_memory_map_entry table[]);

void* memset(void *ptr,  char value, size_t num);
void* memcpy(void *destination, void *source, size_t num);
void memdump(void* src, uint32_t size);

void init_physical_memory_map(uint32_t count, struct boot_memory_map_entry table[]);
void init_heap(struct Free_List_Entry * heap_start, uint8_t* mem_top);

void kfree(void* start);
void* kmalloc(size_t size);

#endif