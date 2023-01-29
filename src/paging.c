#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "paging.h"
#include "mem.h"
#include "terminal.h"
#include "port_io.h"


void set_page_directory_entry(uint32_t index,
                              size_t pte_address,
                              bool page_size, bool rw,
                              bool user, bool write_thru, bool no_caching)
{
    // first, clear the directory entry
    memset(&page_directory[index], 0, sizeof(union Page_Directory_Entry));

    if (page_size)
    {

        page_directory[index].mpage_entry.present = true;
        page_directory[index].mpage_entry.read_write = rw;
        page_directory[index].mpage_entry.user = user;
        page_directory[index].mpage_entry.write_thru = write_thru;
        page_directory[index].mpage_entry.cache_disable = no_caching;
        page_directory[index].mpage_entry.accessed = false;
        page_directory[index].mpage_entry.dirty = false;
        page_directory[index].mpage_entry.page_size = true;
        page_directory[index].mpage_entry.global = false;
        page_directory[index].mpage_entry.available = 0;
        page_directory[index].mpage_entry.page_attribute_table = false;
        page_directory[index].mpage_entry.address_hi = (pte_address >> 20) & 0x1FF;
        page_directory[index].mpage_entry.reserved = false;
        page_directory[index].mpage_entry.address_lo = pte_address & 0xFF;
    }
    else
    {
        page_directory[index].kpage_entry.present = true;
        page_directory[index].kpage_entry.read_write = rw;
        page_directory[index].kpage_entry.user = user;
        page_directory[index].kpage_entry.write_thru = write_thru;
        page_directory[index].kpage_entry.cache_disable = no_caching;
        page_directory[index].kpage_entry.accessed = false;
        page_directory[index].kpage_entry.dirty = false;
        page_directory[index].kpage_entry.page_size = false;
        page_directory[index].kpage_entry.available = 0;
        page_directory[index].kpage_entry.address = pte_address & 0xFFFFF;
    }
}


void set_page_table_entry(uint32_t de,
                          uint32_t te,
                          size_t address,
                          bool page_size, bool rw,
                          bool user, bool write_thru,
                          bool no_caching)
{
    uint16_t index = te + (de * PT_ENTRY_COUNT);
    kprintf("Page dir:table = %x:%x -> index %x\n", de, te, index);


    page_tables[index].fields.present = true;
    page_tables[index].fields.read_write = rw;
    page_tables[index].fields.user = user;
    page_tables[index].fields.write_thru = write_thru;
    page_tables[index].fields.cache_disable = no_caching;
    page_tables[index].fields.accessed = false;
    page_tables[index].fields.dirty = false;
    page_tables[index].fields.page_size = page_size;
    page_tables[index].fields.page_attribute_table = false;
    page_tables[index].fields.global = false;
    page_tables[index].fields.available = 0;
    page_tables[index].fields.address = address;
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
    // corresponding to the virtual address
    uint32_t trailing_directory = ((virt_address % PD_ENTRY_SPAN) != 0) ? 1 : 0;
    uint32_t pd_start = (virt_address / PD_ENTRY_SPAN);
    uint32_t directory_offset = virt_address - (pd_start * PD_ENTRY_SPAN);
    uint32_t pe_start = directory_offset / PAGE_SPAN;

    // determine the page directory entry and page table entry
    // corresponding to the end of the block of memory
    uint32_t block_end = virt_address + block_size - 1;
    uint32_t pd_end = (block_end / PD_ENTRY_SPAN) + trailing_directory;
    uint32_t trailing_block_size = block_size % PD_ENTRY_SPAN;
    uint32_t pe_end = pe_start + (trailing_block_size / PAGE_SPAN) - 1;


    kprintf("physical address: %p, ", phys_address);
    kprintf("virtual address: %p, ", virt_address);
    kprintf("trailing directory: %s\n", (trailing_directory) ? "yes" : "no");
    kprintf("page directory start: %x, ", pd_start);
    kprintf("page table start: %x\n", pe_start);
    kprintf("end address   : %p, ", block_end);
    kprintf("page directory end  : %x, ", pd_end);
    kprintf("page table end  : %x\n", pe_end);
    kprintf("trailing block size : %x\n", trailing_block_size);

    uint32_t pd_entry = pd_start;
    uint32_t pt_entry;
    size_t addr = phys_address;

    for (bool first_entry = true; pd_entry <= pd_end; pd_entry++, first_entry = false)
    {
        pt_entry = first_entry ? pe_start : 0;
        uint32_t pt_current_end = (pd_entry == pd_end) ? pe_end : PT_ENTRY_COUNT;

        set_page_directory_entry(pd_entry,
                                 (size_t) &page_tables[pt_entry],
                                 page_size, rw,
                                 user, write_thru,
                                 no_caching);


        for (; pt_entry <= pt_current_end; pt_entry++, addr += PAGE_SPAN)
        {
            set_page_table_entry(pd_entry,
                                 pt_entry,
                                 addr,
                                 page_size, rw,
                                 user, write_thru,
                                 no_caching);
        }
    }
}


void reset_default_paging(uint32_t map_size, struct memory_map_entry mt[KDATA_MAX_MEMTABLE_SIZE])
{

    // first, set all of the page directory entries to a default state
    memset(&page_directory[0], 0, PD_SIZE);

    // do the same for all of the page table entries
    memset(&page_tables[0], 0, PT_SIZE);

    // identity map the first 1MiB
    set_page_block(0, 0, 0x00100000, false, true, false, false, false);

    // identity map the section for the page directory and page tables
    // these need to have physical addresses, not virtual ones
    set_page_block((size_t) &page_directory, (size_t) &page_directory, 0x00000400, false, true, false, false, false);
    set_page_block((size_t) &page_tables, (size_t) &page_tables, 0x00400000, false, true, false, false, false);

    // map in the kernel region
    set_page_block(0x00100000, (size_t) &kernel_base, 0x00400000, false, true, false, false, false);

    // map in the other various tables
    // provision 4MiB for these just to cover future needs
    set_page_block(0x00400000, (size_t) &tables_base, 0x00400000, false, true, false, false, false);

    // map in the stack
    set_page_block(0x01000000, (size_t) &kernel_stack_base, 0x4000, false, true, false, false, false);


    // reset the paging address control register
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
    );
}