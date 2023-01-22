/* kernel.c */
#include <stdint.h>
#include "kernel.h"
#include "terminal.h"
#include "mem.h"
#include "paging.h"
#include "idt.h"
#include "acpi.h"


void kernel_main()
{
    clear_screen();
    kprints("Starting Kernel...\n", CYAN, BLACK);

    struct kdata* boot_data = (struct kdata*) (KDATA_OFFSET - sizeof(struct kdata) - 16);

    print_mmap(boot_data->mmap_cnt, boot_data->mem_table);

    update_default_paging(boot_data->mmap_cnt, boot_data->mem_table);

    init_default_interrupts();

    init_acpi();

    enable_interrupts();

    kprints("End of kernel services", BLACK, CYAN);
    // panic();
}


