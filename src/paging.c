#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "paging.h"
#include "mem.h"
#include "terminal.h"
#include "port_io.h"


union Page_Table_Entry *page_tables;
union Page_Directory_Entry *page_directory;


//size_t* kernel_physical_base = (size_t *) 0x00100000;

/*
void init_paging_addresses(size_t *mem_top)
{
    page_tables = (union Page_Table_Entry *) ((size_t) kernel_physical_base + kernel_size);
    page_directory = (union Page_Directory_Entry *) ((size_t) page_tables + page_table_size);
    kernel_stack_physical_base = (size_t *) ((size_t) page_directory + page_directory_size);
    tables_physical_base = (size_t *) ((size_t) page_directory + 0x00400000);
    heap_physical_base = (size_t *) ((size_t) tables_physical_base + tables_size);
    heap_size = (size_t) mem_top - (size_t) heap_physical_base;
}
 */


void set_page_directory_entry(uint32_t index,
                              size_t pt_entry,
                              bool rw, bool user,
                              bool write_thru, bool no_caching)
{
    // SANITY CHECK - does this overrun the table?
    if (index > PD_ENTRY_COUNT)
    {
        kprintf("Invalid directory entry index: %x\n", index);
        panic("Invalid directory entry index");
    }

    size_t pte_address = (size_t) &page_tables[pt_entry + (index * PT_ENTRY_COUNT)];
    size_t address_field = pte_address >> 12;

    // clear the directory entry
    memset(&page_directory[index], 0, sizeof(union Page_Directory_Entry));

    //kprintf("directory index %x, ", index);
    //kprintf("Page table entry address: %p, mapped to %x\n", pte_address, address_field);

    page_directory[index].fields.present = true;
    page_directory[index].fields.read_write = rw;
    page_directory[index].fields.user = user;
    page_directory[index].fields.write_thru = write_thru;
    page_directory[index].fields.cache_disable = no_caching;
    page_directory[index].fields.accessed = false;
    page_directory[index].fields.dirty = false;
    page_directory[index].fields.page_size = false;
    page_directory[index].fields.available = 0;
    page_directory[index].fields.address = address_field;
}


void set_page_table_entry(uint32_t de,
                          uint32_t te,
                          size_t address,
                          bool rw, bool user,
                          bool write_thru,
                          bool no_caching)
{
    uint32_t index = te + (de * PT_ENTRY_COUNT);
    size_t address_field = address >> 12;


    // SANITY CHECK - does this overrun the table?
    if (de > PD_ENTRY_COUNT)
    {
        kprintf("Invalid directory entry: %x\n", de);
        panic("Invalid directory entry");
    }

    if (index > (PT_ENTRY_COUNT * PD_ENTRY_COUNT))
    {
        kprintf("Invalid table entry index: %x\n", index);
        panic("Invalid table entry index");
    }

    // first, clear the directory entry
    memset(&page_tables[index], 0, sizeof(union Page_Table_Entry));

    //kprintf("Page dir:table = %x:%x -> index %x, address field: %x\n", de, te, index, address_field);

    page_tables[index].fields.present = true;
    page_tables[index].fields.read_write = rw;
    page_tables[index].fields.user = user;
    page_tables[index].fields.write_thru = write_thru;
    page_tables[index].fields.cache_disable = no_caching;
    page_tables[index].fields.accessed = false;
    page_tables[index].fields.dirty = false;
    page_tables[index].fields.page_attribute_table = false;
    page_tables[index].fields.global = false;
    page_tables[index].fields.available = 0;
    page_tables[index].fields.address = address_field;

    // set matching PM map table entry
    set_pmm_entry(address / PAGE_SPAN);
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
    frame->page_end = (frame->page_start + (block_size / PAGE_SPAN) - 1) % PT_ENTRY_COUNT;

    return frame;
}



/* set a block of page directory and page table entries matching a block of memory */
void set_page_block(uint32_t phys_address,
                    uint32_t virt_address,
                    uint32_t block_size,
                    bool rw, bool user,
                    bool write_thru,
                    bool no_caching)
{
    struct Page_Directory_Frame frame;

    get_frame(&frame, virt_address, block_size);

    // SANITY CHECKS - make sure that the calculated values are sound
    if (frame.dir_start > PD_ENTRY_COUNT)
    {
        kprintf("Invalid directory start: %x\n", frame.dir_start);
        panic("Invalid directory start");
    }

    if (frame.dir_end > PD_ENTRY_COUNT)
    {
        kprintf("Invalid directory endpoint: %x\n", frame.dir_end);
        panic("Invalid directory endpoint");
    }

    if (frame.page_start > PT_ENTRY_COUNT)
    {
        kprintf("Invalid page table entry start: %x\n", frame.page_start);
        panic("Invalid page table entry start");
    }

    if (frame.page_end > PT_ENTRY_COUNT)
    {
        kprintf("Invalid page table entry endpoint: %x\n", frame.page_end);
        panic("Invalid page table entry endpoint");
    }

    // initialize the iteration variables here rather than in the for statement,
    // as the values need to carry over fro one iteration to the next
    uint32_t pd_entry = frame.dir_start;
    uint32_t pt_entry;
    size_t addr = phys_address;


    for (bool first_entry = true; pd_entry <= frame.dir_end; pd_entry++, first_entry = false)
    {
        // if this is the first iteration of the loop, use the computed page entry location, 
        // otherwise start from the beginning of the page entry
        pt_entry = first_entry ? frame.page_start : 0;

        // if this is the final iteration of the loop, use the computed page end location,
        // otherwise fill the whole page entry
        uint32_t pt_current_end = (pd_entry == frame.dir_end) ? frame.page_end + 1 : PT_ENTRY_COUNT;


        // SANITY CHECK - does this overrun the table?
        if (pt_current_end > PT_ENTRY_COUNT)
        {
            kprintf("Invalid local page table entry endpoint: %x\n", pt_current_end);
            panic("Invalid local page table entry endpoint");
        }

        //kprintf("Mapping PD entry %x to physical address: %p\n", pd_entry, addr);

        set_page_directory_entry(pd_entry,
                                 pt_entry,
                                 rw, user,
                                 write_thru,
                                 no_caching);


        for (; pt_entry < pt_current_end; pt_entry++, addr += PAGE_SPAN)
        {
            //kprintf("Mapping %x:%x to physical address: %p\n", pd_entry, pt_entry, addr);
            set_page_table_entry(pd_entry,
                                 pt_entry,
                                 addr,
                                 rw, user,
                                 write_thru,
                                 no_caching);
        }
    }
}

size_t *reset_default_paging(uint8_t *mem_start, uint8_t *mem_top)
{
    size_t* kernel_physical_base = (size_t *) 0x00100000;
    size_t kernel_size = 0x00300000;                             // 3 MiB
    page_tables = (union Page_Table_Entry *) ((size_t) kernel_physical_base + kernel_size);
    size_t page_tables_size = 0x01000000;                         // 16 MiB
    page_directory = (union Page_Directory_Entry *) ((size_t) page_tables + page_tables_size);
    size_t page_directory_size = 0x000001000;                    // 4 KiB
    size_t* kernel_stack_physical_base = (size_t *) ((size_t) page_directory + page_directory_size);
    size_t kernel_stack_size = 0x00004000;                       // 16 KiB
    size_t* tables_physical_base = (size_t *) ((size_t) page_directory + 0x00400000);
    size_t tables_size = 0x00400000;                             // 4 MiB
    size_t* heap_physical_base = (size_t *) ((size_t) tables_physical_base + tables_size);
    size_t heap_size = (size_t) mem_top - (size_t) heap_physical_base;

    kprintf("page tables @%p, %u bytes\n", page_tables, page_tables_size);
    kprintf("page directory @%p, %u bytes\n", page_directory, page_directory_size);
    kprintf("Tables @%p phys, @%p va, %u bytes, top @%p\n", tables_physical_base, &tables_base, tables_size, &tables_top);
    kprintf("Stack @%p phys, @%p va, %u bytes, top @%p\n", kernel_stack_physical_base, &kernel_stack, kernel_stack_size, &kernel_stack_top);


    // identity map the first 1MiB
    set_page_block(0, 0, 0x00100000, true, false, false, false);

    // identity map the kernel region
    set_page_block((size_t) kernel_physical_base, (size_t) kernel_physical_base, kernel_size, true, false, false, false);

    // identity map the section for the page directory and page tables
    // these need to have physical addresses, not virtual ones
    set_page_block((size_t) page_tables, (size_t) page_tables, page_tables_size,  true, false, false, false);
    set_page_block((size_t) page_directory, (size_t) page_directory, page_directory_size, true, false, false, false);

    // identity map the stack
    set_page_block((size_t) kernel_stack_physical_base, (size_t) kernel_stack_physical_base, kernel_stack_size, true, false, false, false);

    // identity map the free heap
    set_page_block((size_t) heap_physical_base, (size_t) heap_physical_base, heap_size, true, false, false, false);


    // map in the kernel region
    set_page_block((size_t) kernel_physical_base, (size_t) &kernel_base, kernel_size, true, false, false, false);

    // map in the other various tables
    // provision 4MiB for these just to cover future needs
    set_page_block((size_t) tables_physical_base, (size_t) &tables_base, tables_size, true, false, false, false);

    // map in the stack
    set_page_block((size_t) kernel_stack_physical_base, (size_t) &kernel_stack, kernel_stack_size, true, false, false, false);

    page_reset();

    return heap_physical_base;
}