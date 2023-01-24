#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "paging.h"
#include "mem.h"
#include "terminal.h"
#include "port_io.h"


void set_page_directory_entry(union Page_Directory_Entry* entry,
                              uint32_t address,
                              bool page_size, bool rw,
                              bool user, bool write_thru, bool no_caching)
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
        entry->mpage_entry.page_size = true;
        entry->mpage_entry.global = false;
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
        entry->mpage_entry.page_size = false;
        entry->kpage_entry.available = 0;
        entry->kpage_entry.address = address & 0xFFFFF;
    }
}


void set_page_table_entry(union Page_Table_Entry* entry,
                          uint32_t address,
                          bool page_size, bool rw,
                          bool user, bool write_thru,
                          bool no_caching)
{
    // first, clear the entry
    memset(entry, 0, sizeof(union Page_Table_Entry));

    entry->fields.present = true;
    entry->fields.read_write = rw;
    entry->fields.user = user;
    entry->fields.write_thru = write_thru;
    entry->fields.cache_disable = no_caching;
    entry->fields.accessed = false;
    entry->fields.dirty = false;
    entry->fields.page_size = page_size;
    entry->fields.page_attribute_table = false;
    entry->fields.global = false;
    entry->fields.available = 0;
    entry->fields.address = address;
}


/* set a block of page directory and page table entries to a common set of values */
void set_page_block(uint32_t start_page,
                    uint32_t address,
                    bool page_size, bool rw,
                    bool user, bool write_thru,
                    bool no_caching,
                    size_t page_count)
{
    uint16_t pde_count = (page_count / PD_SIZE) + 1;
    uint16_t remaining_pte_count = (page_count % PD_SIZE);

    uint32_t dir_addr = address;
    uint32_t pd_start = start_page / PD_SIZE;

    for (uint32_t pd_e = pd_start; pd_e < pde_count; pd_e++, dir_addr += PD_SIZE)
    {
        set_page_directory_entry(&page_directory[pd_e],
                                 dir_addr,
                                 page_size, rw,
                                 user, write_thru,
                                 no_caching);

        uint32_t page_addr = dir_addr;
        for (uint32_t pt_e = 0; pt_e < PT_SIZE; pt_e++, page_addr += PT_SIZE)
        {
            set_page_table_entry(&page_tables[pd_e + pt_e],
                                 page_addr,
                                 page_size, rw,
                                 user, write_thru,
                                 no_caching);
        }
    }

    // finish the remaining page table entries
    uint32_t remaining_page_addr = dir_addr;
    for (uint32_t pt_e = 0; pt_e < remaining_pte_count; pt_e++, remaining_page_addr += PT_SIZE)
    {
        set_page_table_entry(&page_tables[pde_count - 1 + pt_e],
                             remaining_page_addr,
                             page_size, rw,
                             user, write_thru,
                             no_caching);
    }
}


void reset_default_paging(uint32_t map_size, struct memory_map_entry mt[KDATA_MAX_MEMTABLE_SIZE])
{
    kprintf("Clearing page directory at %p\n", page_directory);
    // first, set all of the page directory entries to a default state
    memset(page_directory, 0, 0x1000);

    kprintf("Clearing page tables starting at %p\n", page_tables);
    // do the same for all of the page table entries
    memset(page_tables, 0, 0x400000);

    // next, identity map the first 1MiB
    set_page_block(0, 0, false, true, false, false, false, 0x100000 / PT_SIZE);
    // map in the kernel region
    set_page_block(KERNEL_BASE / PD_SIZE, KERNEL_BASE, false, true, false, false, false, KERNEL_BASE / PT_SIZE);

    __asm__ __volatile__ (
    "    mov %0, %%cr3"
    :
    : "a" (page_directory)
    );

/*     __asm__ __volatile__ (
    "    mov %cr0, %%eax"
    "    or $0x80000000, %%eax"
    "    mov %%eax, %cr0"
    ); */
}