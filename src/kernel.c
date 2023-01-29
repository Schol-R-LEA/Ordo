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

void kernel_main()
{
    clear_screen();
    kprints("Starting Kernel...\n", CYAN, BLACK);


    struct kdata* _boot_data = (struct kdata*) (KDATA_OFFSET - sizeof(struct kdata) - 16);

    set_fg(GRAY);

    kprintf("Moving the memory map\n");
    memcpy(&boot_data, _boot_data, sizeof(struct kdata));
    print_mmap(boot_data.mmap_cnt, boot_data.mem_table);


    kprintf("\nResetting GDT\n");
    reset_gdt();

    reset_default_paging(boot_data.mmap_cnt, boot_data.mem_table);

/*     init_default_interrupts();
    enable_interrupts();

    init_acpi(); */


    kprints("End of kernel services", BLACK, CYAN);
    panic();
}


