#ifndef PAGING_H
#define PAGING_H

#include "kernel.h"
#include "mem.h"

#define PAGE_ADDRESS_MASK 0xfffff000


#define PD_ENTRY_COUNT 0x0400
#define PT_ENTRY_COUNT 0x0400
#define PT_ENTRY_TOTAL_COUNT (PD_ENTRY_COUNT * PT_ENTRY_COUNT)


#define PAGE_SPAN 0x1000
#define PD_ENTRY_SPAN (PT_ENTRY_COUNT * PAGE_SPAN)

#define PD_SIZE (PD_ENTRY_COUNT * sizeof(union Page_Directory_Entry))
#define PT_SIZE (PT_ENTRY_TOTAL_COUNT * sizeof(union Page_Table_Entry))


struct Page_Directory_Entry_Fields
{
    bool present:1;
    bool read_write:1;
    bool user:1;
    bool write_thru:1;
    bool cache_disable:1;
    bool accessed:1;
    bool dirty:1;
    bool page_size:1;
    uint8_t available:4;
    uint32_t address:20;
}__attribute__((packed));


union Page_Directory_Entry
{
    uint32_t raw_entry;
    struct Page_Directory_Entry_Fields fields;
}__attribute__((packed));


struct Page_Table_Entry_fields
{
        bool present:1;
        bool read_write:1;
        bool user:1;
        bool write_thru:1;
        bool cache_disable:1;
        bool accessed:1;
        bool dirty:1;
        bool page_attribute_table:1;
        bool global:1;
        uint8_t available:3;
        uint32_t address:20;
}__attribute__((packed));


union Page_Table_Entry
{
    uint32_t raw_entry;
    struct Page_Table_Entry_fields fields;
}__attribute__((packed));


extern union Page_Table_Entry *page_tables;
extern union Page_Directory_Entry *page_directory;

void set_page_directory_entry(uint32_t index, size_t pt_entry, bool rw, bool user, bool write_thru, bool no_caching);

void set_page_table_entry(uint32_t de, uint32_t te, void* address, bool rw, bool user, bool write_thru, bool no_caching);

void set_page_block(void* phys_address, void* virt_address, uint32_t block_size, bool rw, bool user, bool write_thru, bool no_caching);

void reset_default_paging(size_t heap_size);


inline static void page_reset(void)
{
    // reset the paging address control register
    // to point to the new page directory
     __asm__ __volatile__ (
    "    mov %0, %%cr3"
    :
    : "a" (page_directory)
    : "memory"
    );
}

#endif