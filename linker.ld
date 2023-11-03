/* The bootloader will look at this image and start execution at the symbol
   designated at the entry point. */
ENTRY(kstart)


INPUT(
	obj/kernel.o
	obj/cpu.o
	obj/terminal.o
	obj/mem.o
	obj/idt.o
	obj/gdt.o
	obj/gdt_set.o
    obj/paging.o
	obj/acpi.o
	obj/timer.o
)


OUTPUT(obj/kernel.elf)

OUTPUT_FORMAT(elf32-i386)
STARTUP(obj/kstart.o)

/* Tell where the various sections of the object files will be put in the final
   kernel image. */
SECTIONS
{
    . = 0x000100000;
	PROVIDE(kernel_physical_base = .);

    . = kernel_physical_base + (64K - 16 + 8K);

    .physical_boot_data : ALIGN (4K)
    {
        PROVIDE(kernel_boot_data_physical_base = .);
		. = . + (16 + 8K);
    }
	PROVIDE(kernel_boot_data_physical_end = .);
	PROVIDE(kernel_boot_data_physical_size = . - kernel_boot_data_physical_base);


	/* place the identity-mapped section for
	   the page directory and page tables at the 4MiB
	   point in physical memory (i.e., past the
	   1MiB low memory plus the 3MiB kernel space). */
    . = 0x000400000;
    PROVIDE(kernel_physical_size = . - kernel_physical_base);

    .paging : ALIGN(4K)
	{
		PROVIDE(page_tables_base = .);
		. = . + (4M);
		PROVIDE(page_tables_size = . - page_tables_base);
		PROVIDE(page_directory_base = .);
        . = . + (4K);
		PROVIDE(page_directory_size = . - page_directory_base);
	}

    .physical_gdt : ALIGN(4K)
	{
		PROVIDE(gdt_physical_base = .);
		. = . + (64K);
        PROVIDE(gdt_physical_size = . - gdt_physical_base);
	}

    .physical_tss : ALIGN(4K)
	{
		PROVIDE(tss_physical_base = .);
		. = . + (4K);
        PROVIDE(tss_physical_size = . - tss_physical_base);
	}

    .physical_idt : ALIGN(4K)
	{
		PROVIDE(idt_physical_base = .);
		. = . + (4K);
        PROVIDE(idt_physical_size = . - idt_physical_base);
	}

    .physical_pmmap : ALIGN(4K)
	{
		PROVIDE(pmmap_physical_base = .);
		. = . + (2M);
        PROVIDE(pmmap_physical_size = . - pmmap_physical_base);
	}

    .physical_kernel_stack : ALIGN(4K)
	{
		PROVIDE(kernel_stack_physical_base = .);
		. = . + (4M);
		PROVIDE(kernel_stack_physical_size = . - kernel_stack_physical_base);
	}

    system_reserved_end = .;
	PROVIDE(system_reserved_size = system_reserved_end - kernel_physical_base);

	PROVIDE(heap_physical_base = .);


	/* place the kernel at the start of the higher half */
	. = 0xC0000000;

	/* the .text section. */
	.text : ALIGN(4K)
	{
		PROVIDE(kernel_base = .);
		*(.text)
	}

	/* Read-only data. */
	.rodata : ALIGN(4K)
	{
		*(.rodata)
	}

	/* Read-write data (initialized) */
	.data : ALIGN(4K)
	{
		*(.data)
	}

	/* Read-write data (uninitialized) and stack */
	.bss : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}
    PROVIDE(kernel_end = .);


    /* hardware tables */
    . = 0xC0400000;

    .tables :
	{
		PROVIDE(tables_base = .);
	}

    .boot_data : ALIGN (4K)
    {
        PROVIDE(boot_data = .);
		. = . + (8K);
    }

	.gdt : ALIGN (4K)
	{
		PROVIDE(gdt_base = .);
		. = . + (64K);
	}

	.tss : ALIGN(4K)
	{
		PROVIDE(tss_base = .);
		. = . + (4K);
	}

    .idt : ALIGN(4K)
	{
		PROVIDE(idt_base = .);
		. = . + (4K);
	}

	.print_mmap : ALIGN (4K)
    {
		PROVIDE(pmm_table = .);
		. = . + (2M);
		PROVIDE(pmm_table_size = pmmap_physical_size);
	}


    /* set up the kernel stack */
    . = 0xC1000000;

	.stack : ALIGN(4K)
	{
		PROVIDE(kernel_stack_base = .);
		*(.stack)
		PROVIDE(kernel_stack_top = . + (16K));
	}

    . = kernel_stack_top;

    PROVIDE(.heap_base = .);
}