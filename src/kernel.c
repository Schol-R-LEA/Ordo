/* kernel.c */
#include "terminal.h"

#define KDATA_OFFSET 0xc000fffc

enum KData
{
    mmap_cnt = 0,
    mmap = mmap_cnt + 1 + (24 * 16),
    drive_id,
    fat
};


void kernel_main()
{
    clear_screen();
    kprints("Starting Kernel...\n\n", CYAN, BLACK, 0);

    uint32_t* fs_data = (uint32_t *) KDATA_OFFSET;
    kprints("Drive ID: ", WHITE, BLACK, 0);
    kprintu(*(fs_data - mmap_cnt), 16, WHITE, BLACK, 0);
    kprints("\n\nMemory map (", WHITE, BLACK, 0);
    kprintu(*(fs_data - mmap_cnt), 16, WHITE, BLACK, 0);
    kprints(" entries):\n", WHITE, BLACK, 0);
}