#ifndef MEM_H
#define MEM_H

#include <stddef.h>
#include <stdint.h>

struct memory_map_entry
{
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t ext;
} __attribute__ ((packed));


void print_mmap(uint32_t count, struct memory_map_entry table[]);


void* memset(void *ptr,  char value, size_t num);
void* memcpy(void *destination, void *source, size_t num);


#endif