#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "paging.h"
#include "mem.h"
#include "terminal.h"
#include "port_io.h"


union Page_Table_Entry *page_tables;
union Page_Directory_Entry *page_directory;


void set_page_directory_entry(uint32_t index,
                              size_t pte_address,
                              bool page_size, bool rw,
                              bool user, bool write_thru, bool no_caching)
{
    // first, clear the directory entry
    // memset(&page_directory[index], 0, sizeof(union Page_Directory_Entry));

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
    uint32_t index = te + (de * PT_ENTRY_COUNT);
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

struct Page_Directory_Frame
{
    size_t dir_start, dir_end, page_start, page_end;
};


struct Page_Directory_Frame* get_frame(struct Page_Directory_Frame *frame, size_t virt_address, size_t block_size)
{
    // determine the page directory entry and page table entry
    // corresponding to the virtual address
    bool trailing_directory = ((virt_address % PD_ENTRY_SPAN) != 0) ? true : false;
    frame->dir_start = virt_address / PD_ENTRY_SPAN;
    uint32_t directory_offset = virt_address - (frame->dir_start * PD_ENTRY_SPAN);
    frame->page_start = directory_offset / PAGE_SPAN;

    // determine the page directory entry and page table entry
    // corresponding to the end of the block of memory
    uint32_t block_end = virt_address + block_size - 1;
    frame->dir_end = (block_end / PD_ENTRY_SPAN) + (trailing_directory ? 1 : 0);

    //bool trailing_block_size = block_size % PD_ENTRY_SPAN;
    //trailing_block_size = (trailing_block_size == block_size) ? 0 : trailing_block_size;
    frame->page_end = (frame->page_start + (block_size / PAGE_SPAN) - 1) % PAGE_SPAN;

    kprintf("virtual address: %p, ", virt_address);
    kprintf("trailing directory: %s\n", (trailing_directory) ? "yes" : "no");
    kprintf("page directory start: %x, ", frame->dir_start);
    kprintf("page table start: %x\n", frame->page_start);
    kprintf("end address   : %p, ", block_end);
    kprintf("page directory end  : %x, ", frame->dir_end);
    kprintf("page table end  : %x\n\n", frame->page_end);
    // kprintf("trailing block %s, trailing block size : %x, trailing page %s\n", (trailing_block) ? "yes" : "no", trailing_block_size, (trailing_page) ? "yes" : "no");

    return frame;
}



/* set a block of page directory and page table entries matching a block of memory */
void set_page_block(uint32_t phys_address,
                    uint32_t virt_address,
                    uint32_t block_size,
                    bool page_size, bool rw,
                    bool user, bool write_thru,
                    bool no_caching)
{
    struct Page_Directory_Frame frame;

    get_frame(&frame, virt_address, block_size);

    uint32_t pd_entry = frame.dir_start;
    uint32_t pt_entry;
    size_t addr = phys_address;

    for (bool first_entry = true; pd_entry <= frame.dir_end; pd_entry++, first_entry = false)
    {
        pt_entry = first_entry ? frame.page_start : 0;
        uint32_t pt_current_end = (pd_entry == frame.dir_end) ? frame.page_end : PT_ENTRY_COUNT;

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
    size_t* kernel_physical_base = (size_t *) 0x00100000;
    page_tables = (union Page_Table_Entry *) 0x00400000;
    page_directory = (union Page_Directory_Entry *) 0x01400000;
    size_t* kernel_stack_physical_base = (size_t *) 0x01401000;
    size_t* tables_physical_base = (size_t *) 0x01800000;

    // first, set all of the page directory entries to a default state
    //memset(&page_directory[0], 0, PD_SIZE);

    // do the same for all of the page table entries
    //memset(&page_tables[0], 0, PT_SIZE);

    // identity map the first 1MiB
    set_page_block(0, 0, 0x00100000, false, true, false, false, false);

    // identity map the kernel
    set_page_block((size_t) kernel_physical_base, (size_t) kernel_physical_base, 0x00300000, false, true, false, false, false);

    // identity map the section for the page directory and page tables
    // these need to have physical addresses, not virtual ones
    set_page_block((size_t) page_tables, (size_t) page_tables, 0x01000000, false, true, false, false, false);
    set_page_block((size_t) page_directory, (size_t) page_directory, 0x00000400, false, true, false, false, false);

    // identity map in the stack
    set_page_block((size_t) kernel_stack_physical_base, (size_t) kernel_stack_physical_base, 0x4000, false, true, false, false, false);

    // map in the kernel region
    set_page_block((size_t) kernel_physical_base, (size_t) &kernel_base, 0x00300000, false, true, false, false, false);

    // map in the other various tables
    // provision 4MiB for these just to cover future needs
    set_page_block((size_t) tables_physical_base, (size_t) &tables_base, 0x00400000, false, true, false, false, false);

    // map in the stack
    set_page_block((size_t) kernel_stack_physical_base, (size_t) &kernel_stack_base, 0x4000, false, true, false, false, false);


    // reset the paging address control register
    // to point to the new page directory
    __asm__ __volatile__ (
    "    mov %0, %%cr3"
    :
    : "a" (page_directory)
    : "memory"
    );

    // confirm that the paging bit is set
    // in the main control register
    uint32_t temp;

    __asm__ __volatile__ (
    "    mov %%cr0, %0;"
    "    or $0x80000000, %0;"
    "    mov %0, %%cr0;"
    : "=r"(temp)
    :
    : "memory"
    );
}