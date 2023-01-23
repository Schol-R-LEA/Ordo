#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "paging.h"
#include "mem.h"
#include "port_io.h"

const uint32_t kernel_base_page = KERNEL_BASE / 0x1000;

union Page_Directory_Entry *set_page_directory_entry(union Page_Directory_Entry* entry, uint32_t address, bool page_size, bool rw, bool user, bool write_thru, bool no_caching, bool global)
{
    // first, clear the directory entry
    memset(page_directory, 0, sizeof(union Page_Directory_Entry));

    if (page_size)
    {

        entry->mpage_entry.present = 1;
        entry->mpage_entry.read_write = rw;
        entry->mpage_entry.user = user;
        entry->mpage_entry.write_thru = write_thru;
        entry->mpage_entry.cache_disable = no_caching;
        entry->mpage_entry.accessed = false;
        entry->mpage_entry.dirty = false;
        entry->mpage_entry.page_size = page_size;
        entry->mpage_entry.global = global;
        entry->mpage_entry.available = 0;
        entry->mpage_entry.page_attribute_table = false;
        entry->mpage_entry.address_hi = (address >> 20) & 0x1FF;
        entry->mpage_entry.reserved = false;
        entry->mpage_entry.address_lo = address & 0xFF;
    }
    else
    {
        entry->kpage_entry.present = 1;
        entry->kpage_entry.read_write = rw;
        entry->kpage_entry.user = user;
        entry->kpage_entry.write_thru = write_thru;
        entry->kpage_entry.cache_disable = no_caching;
        entry->kpage_entry.accessed = false;
        entry->kpage_entry.dirty = false;
        entry->mpage_entry.page_size = page_size;
        entry->kpage_entry.available = 0;
        entry->kpage_entry.address = address & 0xFFFFF;
    }

    return page_directory;
}


union Page_Table_Entry* set_page_table_entry(union Page_Table_Entry* entry)
{
    // first, clear the entry
    memset(entry, 0, sizeof(union Page_Table_Entry));


    return entry;
}


void reset_default_paging(uint32_t map_size, struct memory_map_entry mt[KDATA_MAX_MEMTABLE_SIZE])
{
    set_page_directory_entry(page_directory, 0, false, true, false, false, false, false);

    // first, identity map the first 1MiB
    for (int i = 0; i < LOW_MEMORY_PAGE_COUNT; i++)
    {
        set_page_table_entry(&page_tables[i]);
    }

    // next, map the physical location of the kernel code
    for (int i = KERNEL_BASE; i < (KERNEL_BASE + KDATA_OFFSET); i++)
    {
        set_page_table_entry(&page_tables[i]);
    }

}