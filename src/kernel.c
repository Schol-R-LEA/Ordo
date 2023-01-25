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

extern struct kdata *boot_data;

void kernel_main()
{
    clear_screen();
    kprints("Starting Kernel...\n", CYAN, BLACK);

    struct kdata* _boot_data = (struct kdata*) (KDATA_OFFSET - sizeof(struct kdata) - 16);

    set_fg(GRAY);

    kprintf("Moving the memory map\n");
    memcpy(boot_data, _boot_data, sizeof(struct kdata));
    kprintf("boot data address: %p, boot data table size: %d\n", &boot_data, boot_data->mmap_cnt);
    memdump(boot_data->mem_table, 64);

    init_default_interrupts();

    kprintf("\nResetting GDT");
    reset_gdt();

    kprintf("boot data address: %p, boot data table size: %d\n", &boot_data, boot_data->mmap_cnt);
    memdump(boot_data->mem_table, 64);
//    reset_default_paging(boot_data->mmap_cnt, boot_data->mem_table);


    print_mmap(boot_data->mmap_cnt, boot_data->mem_table);

    init_default_interrupts();
    // enable_interrupts();

//    init_acpi();


    kprints("End of kernel services", BLACK, CYAN);
    panic();
}


