#ifndef KERNEL_H
#define KERNEL_H

#include "mem.h"
#include "terminal.h"

#define KERNEL_BASE 0xC0000000
#define KERNEL_PHYS_BASE 0x00010000

#define KDATA_OFFSET (KERNEL_BASE + 0x10000)
#define KDATA_FAT_SIZE (9 * 512)
#define KDATA_MAX_MEMTABLE_SIZE 16

#define KERNEL_CODE_PAGE_COUNT (0x10000 / 0x1000)


struct kdata
{
    uint32_t drive_id;
    uint8_t fat[KDATA_FAT_SIZE];
    struct boot_memory_map_entry mem_table[KDATA_MAX_MEMTABLE_SIZE];
    uint32_t mmap_cnt;
} __attribute__((packed));


enum PRIVILEGE_LEVEL
{
    RING_0, RING_1, RING_2, RING_3
};

extern void *kernel_physical_size, *kernel_physical_base, *kernel_base, *kernel_end,
            *kernel_boot_data_physical_size, *kernel_boot_data_physical_base,
            *page_tables_size, *page_tables_base, *page_directory_size, *page_directory_base,
            *gdt_physical_size, *gdt_physical_base, *gdt_base,
            *tss_physical_size, *tss_physical_base, *tss_base,
            *idt_physical_size, *idt_physical_base, *idt_base,
            *pmmap_physical_size, *pmmap_physical_base, *pmm_base,
            *kernel_stack_physical_size, *kernel_stack_physical_base, *kernel_stack_base,
            *system_reserved_size, *system_reserved_end,
            *heap_physical_base, *heap_base;


static inline void panic(char* msg)
{
    kprintf("%s\n", msg);
    __asm__ ("local_loop:\n"
             "    hlt\n"
             "    jmp local_loop");
}

#endif