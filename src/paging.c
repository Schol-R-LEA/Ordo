#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "paging.h"
#include "mem.h"
#include "terminal.h"
#include "port_io.h"


void set_page_directory_entry(uint32_t index,
                              uint32_t address,
                              bool page_size, bool rw,
                              bool user, bool write_thru, bool no_caching)
{
    // first, clear the directory entry
    memset(page_directory, 0, sizeof(union Page_Directory_Entry));

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
        page_directory[index].mpage_entry.address_hi = (address >> 20) & 0x1FF;
        page_directory[index].mpage_entry.reserved = false;
        page_directory[index].mpage_entry.address_lo = address & 0xFF;
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
        page_directory[index].kpage_entry.address = address & 0xFFFFF;
    }
}


void set_page_table_entry(uint32_t de,
                          uint32_t te,
                          uint32_t address,
                          bool page_size, bool rw,
                          bool user, bool write_thru,
                          bool no_caching)
{

    uint16_t index = te + (de * PT_ENTRY_COUNT);
    // kprintf("directory entry: %x:%x -> index: %x\n", de, te, index);

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
    uint32_t pd_start = virt_address / PD_ENTRY_SPAN;
    uint32_t directory_offset = virt_address - (pd_start * PD_ENTRY_SPAN);
    uint32_t pe_start = directory_offset / PAGE_SPAN;

    // determine the page directory entry and page table entry
    // corresponding to the end of the block of memory
    uint32_t block_end = virt_address + block_size - 1;
    uint32_t pd_end = block_end / PD_ENTRY_SPAN;
    uint32_t trailing_block_size = block_size % PD_ENTRY_SPAN;
    uint32_t pe_end = pe_start + (trailing_block_size / PAGE_SPAN) - 1;


    kprintf("physical address: %p\n", phys_address);
    kprintf("virtual address: %p, ", virt_address);
    kprintf("page directory start: %x, ", pd_start);
    kprintf("page table start: %x\n", pe_start);
    kprintf("end address   : %p, ", block_end);
    kprintf("page directory end  : %x, ", pd_end);
    kprintf("page table end  : %x\n", pe_end);
    kprintf("trailing block size : %x\n", trailing_block_size);


    size_t addr = phys_address;
    for (uint32_t pd_e = pd_start; pd_e < pd_end; pd_e++)
    {
        set_page_directory_entry(pd_e,
                                 addr,
                                 page_size, rw,
                                 user, write_thru,
                                 no_caching);


        for (uint32_t pt_e = pe_start; pt_e < PT_ENTRY_COUNT; pt_e++, addr += PAGE_SPAN)
        {
            set_page_table_entry(pd_e,
                                 pt_e,
                                 addr,
                                 page_size, rw,
                                 user, write_thru,
                                 no_caching);
        }
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
    set_page_block(0, 0, 0x00100000, false, true, false, false, false);

    // map in the kernel region
    set_page_block(0x00100000, KERNEL_BASE, 0x00100000, false, true, false, false, false);

    // map in the various tables
    set_page_block(0x00400000, (size_t) tables_base, 0x01000000, false, true, false, false, false);

    // map in the stack
    set_page_block(0x01400000, (size_t) &kernel_stack_base, 0x4000, false, true, false, false, false);


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