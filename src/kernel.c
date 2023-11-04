/* kernel.c */
#include <stdint.h>
#include "kernel.h"
#include "consts.h"
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

void kernel_main()
{

    size_t total_mem, *mem_top, heap_size;
    clear_screen();
    kprints("Starting Kernel...\n", CYAN, BLACK);

    struct kdata *_boot_data = (struct kdata *) (KDATA_OFFSET - sizeof(struct kdata) - 16);
    //struct kdata *_boot_data = (struct kdata *) &kernel_boot_data_physical_base;

    set_fg(GRAY);

    kprintf("\nMoving the memory map\n");
    memcpy(&boot_data, _boot_data, sizeof(struct kdata));
    print_boot_mmap(boot_data.mmap_cnt, boot_data.mem_table);

    kprintf("\nkernel base address %p, kernel end address %p, %u b\n", &kernel_physical_base, &kernel_physical_end, (size_t) &kernel_end - (size_t) &kernel_base);
    kprintf("initial boot data %p, computed location %p, %u b\n", _boot_data, ((uint8_t *) &kernel_boot_data_physical_base - 16), kernel_boot_data_physical_size);
    total_mem = get_total_mem(boot_data.mmap_cnt, boot_data.mem_table);
    kprintf("Total memory: %u MiB\n", total_mem / MBYTE);
    mem_top = get_mem_top(boot_data.mmap_cnt, boot_data.mem_table);
    kprintf("top of free memory: %p\n", mem_top);

    kprintf("kernel code memory footprint %u bytes\n", kernel_effective_size);
    kprintf("boot data @%p, %u bytes\n", &boot_data, sizeof(struct kdata));

    kprintf("page directory @%p, %u bytes\n", page_directory, page_directory_size);
    kprintf("page tables @%p, %u bytes\n", page_tables, page_tables_size);

    kprintf("GDT @%p phys, @%p va, %u bytes\n", gdt_physical_base, &gdt, gdt_physical_size);
    kprintf("TSS @%p phys, @%p va, %u bytes\n", tss_physical_base, &default_tss, tss_physical_size);
    kprintf("IDT @%p phys, @%p va, %u bytes\n", idt_physical_base, &idt, idt_physical_size);

    kprintf("Stack @%p phys, @%p va, %u bytes\n", kernel_stack_physical_base, &kernel_stack, kernel_stack_physical_size);

    heap_size = (size_t) mem_top - (size_t) heap_physical_base;

    kprintf("heap start: %p, heap size: %d MiB\n", heap, heap_size / MBYTE);

    panic("");


    get_cpu_details();
    kprintf("CPU vendor signature: %s\n", cpu_details.vendor_sig);
    kprintf("APIC: %s\n", (cpu_details.features.apic ? "true" : "false"));

    kprintf("\nResetting GDT... ");
    reset_gdt();
    kprintf("GDT reset\n");

    disable_legacy_timer();
    init_default_interrupts();
    enable_interrupts();

    //reset_default_paging(heap_size);

    //size_t pg_count = init_heap(heap_entry_point, mem_top);
    //kprintf("Pages initialized: %u\n", pg_count);

    // init_acpi();

    set_fg(BLACK);
    set_bg(CYAN);
    panic("End of kernel services");
}


