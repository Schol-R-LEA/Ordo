/* kernel.c */
#include <stdint.h>
#include "kernel.h"
#include "cpu.h"
#include "terminal.h"
#include "mem.h"
#include "paging.h"
#include "timer.h"
#include "gdt.h"
#include "tss.h"
#include "idt.h"
#include "acpi.h"

extern struct kdata boot_data;
extern size_t kernel_end;
extern size_t reserved_end;

size_t kernel_size, system_reserved_size, *heap_entry_point;

void kernel_main()
{
    size_t total_mem, *mem_start, *mem_top, heap_size;
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
    mem_start = get_mem_start(boot_data.mmap_cnt, boot_data.mem_table);
    mem_top = get_mem_top(boot_data.mmap_cnt, boot_data.mem_table);
    kprintf("top of free memory: %p\n", mem_top);
    kernel_size = (size_t) &kernel_end - (size_t) &kernel_base;
    system_reserved_size = (size_t) &kernel_end - (size_t) &kernel_base;
    kprintf("kernel memory footprint %u KiB\n", kernel_size / KBYTE);
    init_physical_memory_map(boot_data.mmap_cnt, boot_data.mem_table);
    heap_entry_point = page_round_up(mem_start + system_reserved_size);
    heap_size = (size_t) mem_top - (size_t) heap_entry_point;
    kprintf("system reserved size %x\n", system_reserved_size);
    kprintf("heap start: %p, heap size: %d MiB\n", heap_entry_point, heap_size / MBYTE);

    get_cpu_details();
    kprintf("CPU vendor signature: %s\n", cpu_details.vendor_sig);
    kprintf("APIC: %s\n", (cpu_details.features.apic ? "true" : "false"));

    kprintf("\nResetting GDT... ");
    reset_gdt();
    kprintf("GDT reset\n");

    init_timer();
    init_default_interrupts();
    // enable_interrupts();

    reset_default_paging(boot_data.mmap_cnt, boot_data.mem_table, heap_entry_point, heap_size);

    size_t pg_count = init_heap(heap_entry_point, mem_top);
    kprintf("Pages initialized: %u\n", pg_count);

    init_acpi();

    set_fg(BLACK);
    set_bg(CYAN);
    panic("End of kernel services");
}


