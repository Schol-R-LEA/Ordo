#ifndef PAGING_H
#define PAGING_H

#include "kernel.h"
#include "mem.h"

#define PD_ENTRY_COUNT 0x0400
#define PT_ENTRY_COUNT 0x0400
#define PT_ENTRY_TOTAL_COUNT (PD_ENTRY_COUNT * PT_ENTRY_COUNT)


#define PAGE_SPAN 0x1000
#define PD_ENTRY_SPAN (PT_ENTRY_COUNT * PAGE_SPAN)

#define PD_SIZE (PD_ENTRY_COUNT * sizeof(union Page_Directory_Entry))
#define PT_SIZE (PT_ENTRY_TOTAL_COUNT * sizeof(union Page_Table_Entry))


struct Page_Directory_Entry_kilobyte
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
};


struct Page_Directory_Entry_megabyte
{
    bool present:1;
    bool read_write:1;
    bool user:1;
    bool write_thru:1;
    bool cache_disable:1;
    bool accessed:1;
    bool dirty:1;
    bool page_size:1;
    bool global:1;
    uint8_t available:3;
    bool page_attribute_table:1;
    uint8_t address_hi:8;
    bool reserved:1;
    uint16_t address_lo:10;
};


union Page_Directory_Entry
{
    uint32_t raw_entry;
    struct Page_Directory_Entry_kilobyte kpage_entry;
    struct Page_Directory_Entry_megabyte mpage_entry;
} __attribute__((aligned(8)));


struct Page_Table_Entry_fields
{
        bool present:1;
        bool read_write:1;
        bool user:1;
        bool write_thru:1;
        bool cache_disable:1;
        bool accessed:1;
        bool dirty:1;
        bool page_size:1;
        bool page_attribute_table:1;
        bool global:1;
        uint8_t available:3;
        uint32_t address:19;
};


union Page_Table_Entry
{
    uint32_t raw_entry;
    struct Page_Table_Entry_fields fields;
} __attribute__((aligned(8))) ;


extern union Page_Directory_Entry page_directory[PD_ENTRY_COUNT];
extern union Page_Table_Entry page_tables[PT_ENTRY_TOTAL_COUNT];

void set_page_directory_entry(union Page_Directory_Entry* entry, uint32_t address, bool page_size, bool rw, bool user, bool write_thru, bool no_caching);

void set_page_table_entry(union Page_Table_Entry* entry, uint32_t address, bool page_size, bool rw, bool user, bool write_thru, bool no_caching);

void set_page_table_entry(union Page_Table_Entry* entry, uint32_t address, bool page_size, bool rw, bool user, bool write_thru, bool no_caching);

void reset_default_paging(uint32_t map_size, struct memory_map_entry mt[KDATA_MAX_MEMTABLE_SIZE]);


#endif