/* kernel.c */
#include <stdint.h>
#include "terminal.h"
#include "mem_map.h"

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

    kprintu(sizeof(struct kdata), 10, WHITE, BLACK, 0);

    struct kdata* fs_data = (struct kdata *) (KDATA_OFFSET - sizeof(struct kdata) - 16);

    kprints("\nPassed Data offset ", WHITE, BLACK, 0);
    kprintu((uint32_t)(fs_data), 16, WHITE, BLACK, 0);

    kprints("\ndrive ID offset ", WHITE, BLACK, 0);
    kprintu((uint32_t) &fs_data->drive_id, 16, WHITE, BLACK, 0);

    kprints("\nDrive ID: ", WHITE, BLACK, 0);
    kprintu(fs_data->drive_id, 16, WHITE, BLACK, 0);


    kprints("\nFAT offset ", WHITE, BLACK, 0);
    kprintu((uint32_t) &fs_data->fat, 16, WHITE, BLACK, 0);

    kprints("\nMemory Map Table offset ", WHITE, BLACK, 0);
    kprintu((uint32_t) &fs_data->mem_table, 16, WHITE, BLACK, 0);
    kprints("\nMemory Map size offset ", WHITE, BLACK, 0);
    kprintu((uint32_t) &fs_data->mmap_cnt, 16, WHITE, BLACK, 0);
    print_mmap(fs_data->mmap_cnt, fs_data->mem_table);
}





