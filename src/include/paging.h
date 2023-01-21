#ifndef PAGING_H
#define PAGING_H

struct Page_Directory_Entry_kilobyte
{

    uint32_t present:1;
    uint32_t read_write:1;
    uint32_t super:1;
    uint32_t write_thru:1;
    uint32_t cache_disable:1;
    uint32_t accessed:1;
    uint32_t dirty:1;
    uint32_t page_size:1;
    uint32_t available:4;
    uint32_t address:10;
};


struct Page_Directory_Entry_megabyte
{
    uint32_t present:1;
    uint32_t read_write:1;
    uint32_t super:1;
    uint32_t write_thru:1;
    uint32_t cache_disable:1;
    uint32_t accessed:1;
    uint32_t dirty:1;
    uint32_t page_size:1;
    uint32_t global:1;
    uint32_t available:3;
    uint32_t page_attribute_table:1;
    uint32_t address_hi:8;
    uint32_t reserved:1;
    uint32_t address_lo:10;
};


union Page_Directory_Entry
{
    uint32_t raw_entry;
    struct Page_Directory_Entry_kilobyte kpage_entry;
    struct Page_Directory_Entry_kilobyte mpage_entry;
};


struct Page_Table_Entry_fields
{
        uint32_t present:1;
        uint32_t read_write:1;
        uint32_t super:1;
        uint32_t write_thru:1;
        uint32_t cache_disable:1;
        uint32_t accessed:1;
        uint32_t dirty:1;
        uint32_t page_size:1;
        uint32_t page_attribute_table:1;
        uint32_t global:1;
        uint32_t available:3;
        uint32_t address:19;
};


union Page_Table_Entry
{
    uint32_t raw_entry;
    struct Page_Table_Entry_fields fields;
};


#endif