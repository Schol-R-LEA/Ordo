#ifndef PAGING_H
#define PAGING_H

#include "kernel.h"
#include "mem.h"



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
    uint32_t address_hi:8;
    bool reserved:1;
    uint32_t address_lo:10;
};


union Page_Directory_Entry
{
    uint32_t raw_entry;
    struct Page_Directory_Entry_kilobyte kpage_entry;
    struct Page_Directory_Entry_kilobyte mpage_entry;
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


void update_default_paging(uint32_t map_size, struct memory_map_entry mt[KDATA_MAX_MEMTABLE_SIZE]);


#endif