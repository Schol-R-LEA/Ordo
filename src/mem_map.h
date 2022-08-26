#ifndef MEM_MAP_H
#define MEM_MAP_H

#include <stdint.h>

struct memory_map_entry
{
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t ext;
} __attribute__ ((packed));


void print_mmap(uint32_t count, struct memory_map_entry* table);

#endif