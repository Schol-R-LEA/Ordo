/* kernel.c */
#include <stdint.h>
#include "terminal.h"
#include "mem_map.h"

#define KDATA_OFFSET 0xc000fffc

enum KData
{
    mmap_cnt = 0,
    mmap = mmap_cnt + 1 + (24 * 16),
    drive_id,
    fat = drive_id + 1 + (9 * (512 / 4))
};




void kernel_main()
{
    clear_screen();
    kprints("Starting Kernel...\n\n", CYAN, BLACK, 0);

    uint32_t* fs_data = (uint32_t *) KDATA_OFFSET;

    kprints("Passed Data offset ", WHITE, BLACK, 0);
    kprintu((uint32_t)fs_data, 16, WHITE, BLACK, 0);

    kprints("\nDrive ID: ", WHITE, BLACK, 0);
    kprintu(*(fs_data - drive_id), 16, WHITE, BLACK, 0);
    print_mmap(*(fs_data - mmap_cnt), (struct memory_map_entry*)(fs_data - mmap));
}





