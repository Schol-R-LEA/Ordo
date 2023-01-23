#ifndef KERNEL_H
#define KERNEL_H

#include "mem.h"


#define KERNEL_BASE 0xC0000000

#define KDATA_OFFSET 0xc0010000
#define KDATA_FAT_SIZE (9 * 512)
#define KDATA_MAX_MEMTABLE_SIZE 16


struct kdata
{
    uint32_t drive_id;
    uint8_t fat[KDATA_FAT_SIZE];
    struct memory_map_entry mem_table[KDATA_MAX_MEMTABLE_SIZE];
    uint32_t mmap_cnt;
} __attribute__((packed));


enum PRIVILEGE_LEVEL
{
    RING_0, RING_1, RING_2, RING_3
};


static inline void panic()
{
    __asm__ ("local_loop:\n"
             "    hlt\n"
             "    jmp local_loop");
}

#endif