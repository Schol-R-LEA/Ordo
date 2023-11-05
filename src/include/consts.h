#ifndef CONSTS_H
#define CONSTS_H

#include <stddef.h>
#include <stdint.h>
#include "consts.h"
#include "paging.h"
#include "gdt.h"
#include "tss.h"
#include "idt.h"
#include "mem.h"


// computed physical addresses for various kernel tables
#define page_tables_offset ((union Page_Table_Entry *) 0x000400000)
#define page_directory_offset ((union Page_Directory_Entry *)  ((size_t) page_tables_offset + (16 * MBYTE)))

#define kernel_stack_physical_offset ((size_t *) ((size_t) page_directory_offset + PAGE_SIZE))

#define tables_physical_offset ((void *) ((size_t) kernel_stack_physical_offset + (16 * KBYTE)))
#define gdt_physical_offset ((union GDT_Entry *) tables_physical_offset)
#define tss_physical_offset ((struct TSS *) ((size_t) gdt_physical_offset + (64 * KBYTE)))
#define idt_physical_offset ((struct Interrupt_Descriptor_32 *) ((size_t) tss_physical_offset + PAGE_SIZE))


#define heap_physical_offset ((struct Free_List_Entry *) ((size_t) tables_physical_offset + 4 * MBYTE))



// addresses imported from linker script
// all of these except for kernel_physical_base and kernel_physical_end
// point to locations in the higher half
extern const uint8_t *kernel_physical_base, *kernel_physical_end, *kernel_base, *kernel_end;
extern const uint8_t *tables_base;
extern const union Page_Table_Entry *page_tables;
extern const union Page_Directory_Entry *page_directory;
extern const union GDT_Entry *gdt;
extern const struct TSS *default_tss;
extern const struct Interrupt_Descriptor_32 *idt;
extern const size_t *kernel_stack;
extern const size_t *kernel_stack_top;
extern const struct Free_List_Entry *heap;


extern const size_t kernel_effective_size, kernel_boot_data_physical_size,
                    page_tables_size, page_directory_size,
                    gdt_physical_size, tss_physical_size, idt_physical_size,
                    kernel_stack_physical_size,
                    tables_physical_size;


#endif