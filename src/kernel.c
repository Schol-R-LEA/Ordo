/* kernel.c */
#include <stdint.h>
#include "kernel.h"
#include "terminal.h"
#include "mem.h"
#include "paging.h"
#include "gdt.h"
#include "tss.h"
#include "idt.h"
#include "acpi.h"

extern struct kdata boot_data;
extern size_t kernel_end;

size_t kernel_size;

void kernel_main()
{
    size_t total_mem,*mem_top;
    clear_screen();
    kprints("Starting Kernel...\n", CYAN, BLACK);


    struct kdata* _boot_data = (struct kdata*) (KDATA_OFFSET - sizeof(struct kdata) - 16);

    set_fg(GRAY);

    kprintf("Moving the memory map\n");
    memcpy(&boot_data, _boot_data, sizeof(struct kdata));
    print_boot_mmap(boot_data.mmap_cnt, boot_data.mem_table);

    kprintf("kernel base address %p, kernel end address %p\n", &kernel_base, &kernel_end);
    total_mem = get_total_mem(boot_data.mmap_cnt, boot_data.mem_table);
    kprintf("Total memory: %u MiB\n", total_mem / MBYTE);
    mem_top = get_mem_top(boot_data.mmap_cnt, boot_data.mem_table);
    kprintf("top of free memory: %u\n", (size_t) mem_top);
    kernel_size = (size_t) &kernel_end - (size_t) &kernel_base;
    kprintf("kernel memory footprint %u KiB\n", kernel_size / KBYTE);
    init_physical_memory_map(boot_data.mmap_cnt, boot_data.mem_table);

    init_heap(mem_top);


    kprintf("\nResetting GDT... ");
    reset_gdt();
    kprintf("GDT reset\n");

    reset_default_paging(boot_data.mmap_cnt, boot_data.mem_table);

    init_default_interrupts();
    //enable_interrupts();

    init_acpi();

    set_fg(BLACK);
    set_bg(CYAN);
    panic("End of kernel services");
}


