#include <stddef.h>
#include <stdint.h>
#include "consts.h"
#include "kernel.h"

// provision 3M for the kernel, regardless of its actual size
const size_t kernel_effective_size = 0x00300000;

const size_t kernel_boot_data_physical_size = sizeof(struct kdata);

const size_t page_directory_size = PAGE_SIZE;
const size_t page_tables_size = (16 * MBYTE);

const size_t gdt_physical_size = (16 * KBYTE);
const size_t tss_physical_size = PAGE_SIZE;      // round both TSS and IDT allocations up 
const size_t idt_physical_size = PAGE_SIZE;      // to the size of a page


// kernel stack is set to 16KiB
const size_t kernel_stack_physical_size = (16 * KBYTE);

const size_t tables_physical_size = (4 * MBYTE);