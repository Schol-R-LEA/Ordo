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

        entry->mpage_entry.present = true;
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
        entry->kpage_entry.present = true;
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


/* set a block of page directory and page table entries matching a block of memory */
void set_page_block(uint32_t phys_address,
                    uint32_t virt_address,
                    uint32_t block_size,
                    bool page_size, bool rw,
                    bool user, bool write_thru,
                    bool no_caching)
{
    // determine the page directory entry and page table entry
    // corresponding to the physical address
    uint32_t pd_start = virt_address / PD_ENTRY_SPAN;
    uint32_t directory_offset = virt_address - (pd_start * PD_ENTRY_SPAN);
    uint32_t pe_start = directory_offset / PAGE_SPAN;


    uint32_t block_end = virt_address + block_size - 1;
    uint32_t pd_end = block_end / PD_ENTRY_SPAN;
    uint32_t trailing_block_size = block_size % PD_ENTRY_SPAN;
    uint32_t pe_end = pe_start + (trailing_block_size / PAGE_SPAN) - 1;


    kprintf("physical address: %p\n", phys_address);
    kprintf("target address: %p, ", virt_address);
    kprintf("page directory start: %x, ", pd_start);
    kprintf("page table start: %x\n", pe_start);
    kprintf("end address   : %p, ", block_end);
    kprintf("page directory end  : %x, ", pd_end);
    kprintf("page table end  : %x\n", pe_end);
    kprintf("trailing block size : %x\n", trailing_block_size);


    uint32_t pt_e = pe_start;
    size_t addr = phys_address;

    for (uint32_t pd_e = pd_start; pd_e < pe_end; pd_e++)
    {
        set_page_directory_entry(&page_directory[pd_e],
                                 addr,
                                 page_size, rw,
                                 user, write_thru,
                                 no_caching);


        for (; pt_e < PT_SIZE; pt_e++, addr += PAGE_SPAN)
        {
            set_page_table_entry(&page_tables[pt_e + (pd_e * PAGE_SPAN)],
                                 addr,
                                 page_size, rw,
                                 user, write_thru,
                                 no_caching);
        }
    }

    // finish the remaining page table entries
    for (; pt_e < pe_end; pt_e++, addr += PAGE_SPAN)
    {
        set_page_table_entry(&page_tables[pt_e],
                             addr,
                             page_size, rw,
                             user, write_thru,
                             no_caching);
    }
}


void reset_default_paging(uint32_t map_size, struct memory_map_entry mt[KDATA_MAX_MEMTABLE_SIZE])
{
    kprintf("Clearing page directory at %p\n", page_directory);
    // first, set all of the page directory entries to a default state
    memset(page_directory, 0, PD_SIZE);

    kprintf("Clearing page tables starting at %p\n", page_tables);
    // do the same for all of the page table entries
    memset(page_tables, 0, PT_SIZE);

    // next, identity map the first 1MiB
    set_page_block(0, 0, 0x100000, false, true, false, false, false);

    // map in the kernel region
    set_page_block(0x00100000, KERNEL_BASE, 0x100000, false, true, false, false, false);

    // map in the various tables
    set_page_block(0x00400000, (size_t) tables_base, 0x0C00000, false, true, false, false, false);

    // map in the stack
    set_page_block(0x01500000, (size_t) &kernel_stack_base, 0x4000, false, true, false, false, false);


/*     // reset the paging address control register
    // to point to the new page directory
    __asm__ __volatile__ (
    "    mov %0, %%cr3"
    :
    : "a" (page_directory)
    );

    // confirm that the paging bit is set
    // in the main control register
    __asm__ __volatile__ (
    "    mov %cr0, %eax;"
    "    or $0x80000000, %eax;"
    "    mov %eax, %cr0;"
    ); */
}