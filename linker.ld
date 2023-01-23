/* The bootloader will look at this image and start execution at the symbol
   designated at the entry point. */
ENTRY(kstart)

INPUT(
	obj/kernel.o
	obj/terminal.o
	obj/mem.o
	obj/idt.o
	obj/gdt.o
    obj/paging.o
	obj/acpi.o
)

OUTPUT(kernel.elf)

OUTPUT_FORMAT(elf32-i386)
STARTUP(obj/kstart.o)

/* Tell where the various sections of the object files will be put in the final
   kernel image. */
SECTIONS
{
	/* Begin putting sections at the higher half. */
	. = 0xC0000000;

	/* the .text section. */
	.text : ALIGN(4K)
	{
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

    /* hardware tables */
    . = 0xC0100000;

	.gdt BLOCK(64K) :
	{
		gdt = .;
		. = . + 64K;
	}

	.tss BLOCK(4K) : ALIGN(4K)
	{
		default_tss = .;
		. = . + 4K;
	}

    .idt BLOCK(4K) : ALIGN(4K)
	{
		idt = .;
		. = . + 4K;
	}

    .paging BLOCK(4K) : ALIGN(4K)
	{
        page_directory = .;
        . = . + 4K;
		page_tables = .;
		. = . + (1K * 4K);
	}

    /* set up the kernel stack */
    . = 0xC1000000;

    .stack :
	{
		kernel_stack_base = .;
		kernel_stack_top = . + 16K;
	}
}