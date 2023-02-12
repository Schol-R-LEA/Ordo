#ifndef MEM_H
#define MEM_H

#include <stddef.h>
#include <stdint.h>

struct boot_memory_map_entry
{
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t ext;
} __attribute__ ((packed));


enum PM_Type : uint8_t
{
    NONE, RESERVED, FIRMWARE, MMIO, FREE
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


struct KHM_Entry
{
    size_t *base;
    size_t span;
    struct KHM_Entry *prev;
    struct KHM_Entry *next;
};


extern uint64_t pmm_table[];
extern uint8_t pmm_table_end;



static inline void set_pmm_entry(size_t index)
{
    size_t byte_index = index / 64;
    size_t bit_index = index % 64;

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
size_t *get_mem_top(uint32_t count, struct boot_memory_map_entry table[]);

void* memset(void *ptr,  char value, size_t num);
void* memcpy(void *destination, void *source, size_t num);
void memdump(void* src, uint32_t size);

void init_physical_memory_map(uint32_t count, struct boot_memory_map_entry table[]);
void init_kernel_heap(uint32_t count, struct boot_memory_map_entry table[]);

#endif