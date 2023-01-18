/* kernel.c */
#include <stdint.h>
#include "kernel.h"
#include "terminal.h"
#include "mem_map.h"
#include "idt.h"
#include "acpi.h"


#define KDATA_OFFSET 0xc0010000

struct kdata
{
    uint32_t drive_id;
    uint8_t fat[9 * 512];
    struct memory_map_entry mem_table[16];
    uint32_t mmap_cnt;
} __attribute__((packed));


void kernel_main()
{
    clear_screen();
    kprints("Starting Kernel...\n", CYAN, BLACK, 0);

    struct kdata* fs_data = (struct kdata *) (KDATA_OFFSET - sizeof(struct kdata) - 16);

    print_mmap(fs_data->mmap_cnt, fs_data->mem_table);

    init_default_interrupts();

    init_acpi();

    //enable_interrupts();

    kprints("End of kernel services", BLACK, CYAN, 0);
    panic();
}


