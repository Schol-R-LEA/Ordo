/* The bootloader will look at this image and start execution at the symbol
   designated at the entry point. */
ENTRY(kernel_main)

OUTPUT_FORMAT(elf32-i386)
STARTUP(obj/kstart.o)

/* Tell where the various sections of the object files will be put in the final
   kernel image. */
SECTIONS
{
	/* Begin putting sections at the higher half. */
	. = 0xC0000000;

	/* the .text section. */
	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.text)
	}

	/* Read-only data. */
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
	}

	/* Read-write data (initialized) */
	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}

	/* Read-write data (uninitialized) and stack */
	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}

	/* The compiler may produce other sections, put them in the proper place in
	   in this file, if you'd like to include them in the final kernel. */
}