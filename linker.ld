/* The bootloader will look at this image and start execution at the symbol
   designated at the entry point. */
ENTRY(kstart)


INPUT(
	obj/kernel.o
	obj/consts.o
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


	PROVIDE(kernel_physical_end = . + (3M));


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
		PROVIDE(gdt = .);
		. = . + (64K);
	}

	.tss : ALIGN(4K)
	{
		PROVIDE(default_tss = .);
		. = . + (4K);
	}

    .idt : ALIGN(4K)
	{
		PROVIDE(idt = .);
		. = . + (4K);
	}


    /* set up the kernel stack */
    . = 0xC1000000;

	.stack : ALIGN(4K)
	{
		PROVIDE(kernel_stack = .);
		*(.stack)
		PROVIDE(kernel_stack_top = . + (16K));
	}

    . = kernel_stack_top;

    PROVIDE(heap = .);
}